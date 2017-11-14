/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém inclusão de bibliotecas e namespaces, definição de macros, definição de códigos de erro,
  declaração das classes e das funções utilizadas, definição de tipos*/

/*Bibliotecas Utilizadas no Programa*/
#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

/*Namespaces Utilizados*/
using std::ifstream;
using std::ofstream;
using namespace std;

/*Macros Utilizadas*/
#define MAX_NOME 11					/*Tamanho máximo que o nome de um arquivo .NET pode ter*/
#define MAX_LINHA 80				/*Tamanho máximo que uma linha de um arquivo .NET pode ter*/
#define MAX_COMPONENTE 100			/*Tamanho máximo de componentes em um arquivo .NET*/
#define MAX_NOS 100					/*Tamanho máximo de nós que um circuito, definido em um arquivo .NET, pode ter*/
#define MAX_CORRENTES_EXTRAS 50		/*Máximo número de correntes que o programa pode adicionar a um circuito para calcular correntes em ramos*/
#define TOLG 1e-9					/*Tolerância para determinante, a fim de detectar se um sistema é singular */
#define PI atan(1)*4				/*Definição do clássico número PI*/
#define RESISTOR_DE_GAMBIARRA 1e9	/*Resistor utilizado no cálculo do ponto de operação de circuitos com capacitores e indutores*/

/*Códigos de Erro que o Programa Retorna*/
enum error 
{
	SUCESSO,									/*Caso o programa execute por completo, sem erros*/

	/*Erros ao executar CircuitSimulator.cpp*/
	ERRO_MAIN_NAO_CONVERGENCIA,					/*Caso em que o programa é encerrado por um componente não linear não ter convergido*/
	
	/*Erros ao executar Functions_CircuitSimulator.cpp*/
	ERRO_ARQUIVO_INEXISTENTE,					/*Caso um arquivo .NET, fornecido como entrada para o programa, não exista*/
	ERRO_NUMERO_MAXIMO_ELEMENTOS,				/*Caso o arquivo .NET fornecido como entrada tenha mais de 100 elementos*/
	ERRO_ELEMENTO_DESCONHECIDO,					/*Caso o arquivo .NET fornecido como entrada possua um elemento desconhecido*/
	ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES,	/*Caso o número de correntes adicionadas ao circuito pelo programa exceda um valor máximo*/
	ERRO_ESTAMPAR,								/*Caso tenha-se tentado estampar um componente que não existe*/
	ERRO_NUMERO_MAXIMO_NOS,						/*Caso o arquivo .NET fornecido como entrada descreva um circuito com mais de 100 nós*/
	ERRO_SISTEMA_SINGULAR,							/*Caso, ao resolver um sistema de equações, o programa se deparar com um sistema singular*/
	
	/*Erros ao executar Methods_NewtonRaphson_Algorithm*/
	ESTABILIZOU,								/*Caso uma análise com elementos não-lineares tenha estabilizado*/
	ERRO_DE_ESTABILIZACAO,						/*Caso uma análise com elementos não-lineares não tenha estabilizado*/
	ERRO_ESTAMPAR_NAO_LINEAR,					/*Caso não exista chaves ou resistores não lineares e uma atualização de estampa para elementos não lineares for pedida*/
	ERRO_COMPONENTE_NAO_CALCULADO,				/*Caso não exista chaves ou resistores não lineares e um calculo desses componentes foi solicitado*/

	/*Erros ao executar Methods_Time_Analize*/
};

/*Classe que contém os atributos de um componente genérico que um circuito pode ter*/
class Componente{
public:
	Componente() { nome = ""; valor = 0.0; no_A = 0; no_B = 0; no_C = 0; no_D = 0; j_x = 0; j_y = 0; jc_0 = 0; valor_novo = 0; Io = 0; gmin = 0; }	/*Construtor da classe*/
	string tipo;			/*Qual componente é? R? C? L?*/
	string nome;			/*Nome fantasia do componente. Ex: R0100*/
	double valor;			/*Valor do componente*/

	int no_A;				/*Número do Nó A onde o componente está ligado*/
	int	no_B;				/*Número do Nó B onde o componente está ligado*/
	int	no_C;				/*Se for um AmpOp, número do Nó C onde o componente está ligado OU se for uma fonte controlada(chave), número do nó C do ramo de controle*/
	int	no_D;				/*Se for um AmpOp, número do Nó D onde o componente está ligado OU se for uma fonte controlada(chave), número do nó D do ramo de controle*/

	int	j_x;				/*Número da coluna/linha de uma matriz correspondente à uma corrente X*/
	int	j_y;				/*Número da coluna/linha de uma matriz correspondente à uma corrente Y*/

	double valor_novo;		/*Variável auxiliar para armezenar um valor*/
	double Io;				/*Variável auxiliar para armezenar um valor inicial de corrente em Newton-Raphson*/						
	double jc_0;			/*Variável auxiliar para armezenar um valor de corrente em capacitores*/

	double gmin;			/*Valor de condutância utilizada no processo de Gmin stepping*/
};

/*Tipos de Variável Utilizados*/
typedef vector<vector<double>> matriz;	/*Uma matriz onde todos os valores podem possuir casas decimais*/
typedef vector<vector<string>> infcomp; /*Uma matriz onde todos os valores são strings*/
typedef vector<Componente> netlist;		/*Um vetor de elementos da classe componente*/

/*Classe que contém instruções para uma análise no tempo*/
class Dados_Analise {
public:
	Dados_Analise() { tipo_Analise = ".TRAN"; tempo_Atual = 0; tempo_Final = 1; passo = 1; metodo = "TRAP"; passos_Tabela = 1; } /*Construtor da classe*/
	string tipo_Analise;			/*Armazena a informação .TRAN, que diz ao programa que uma análise no tempo deve ser feita*/
	double tempo_Atual;				/*Tempo t em que a análise está sendo realizada*/
	double tempo_Final;				/*Tempo de duração da análise*/
	double passo;					/*Tempo entre um cálculo e outro durante a análise. Ex: O programa calcula uma tensão para t e para t+passo*/
	string metodo;					/*Método da análise. Ex: TRAP, BE, FE*/
	int passos_Tabela;
	size_t numero_De_Analises;		/*Quantidade de análises no tempo que devem ser feitas. tempo total / passo*/
	infcomp comp_var;				/*Matriz que contém informações de um componente variável no tempo. Ex: Fontes senoidais*/
	vector<size_t> posicao_var;
	
	/*Métodos que pertencem à classe*/
	int CalcularComponentesTempo(netlist &net_List);									/*Calcula o valor das fontes pulsantes e senoidais em cada instante de tempo*/
	int AtualizarEstampa(netlist &net_List, matriz &sistema, matriz sistema_Anterior);	/*Atualiza a estampa dos componentes variáveis no tempo durante a análise*/
	int NumeroDeOperacoes();															/*Calcula quantas análises no tempo devem ser realizadas*/
};

/*Classe que contém informações de componentes não lineraes*/
class Dados_NR {
public:
	infcomp comp_var;				/*Matriz que contém os parâmetros de elementos não lineares*/
	vector<size_t> posicao_var;

	/*Métodos que pertencem à classe*/
	int CalcularNewtonRaphson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior);			/*Calcula a próxima aproximação quando no Método de Newton-Raphson*/
	int EstampaNR(matriz &sistema, netlist &net_list, char tipo, size_t indice, double novo_valor); /*Atualiza a estampa dos componentes não lineares durante a análise*/
	double CalcularValorNR(vector<string> paramNR, double valorAnterior, double &I0);				/*Calcula o próximo valor de um componente não linear*/
	int GminStep(matriz &sistema, netlist &net_List, char tipo, size_t indice, bool convergencia, double &fator);
	size_t InteracaoNR(matriz &sistema, netlist &net_List, matriz &sistema_Anterior, vector<bool> &verifica_Convergencia);
};

/*Classe que armazena as informações a serem impressas no arquivo .TAB de saída*/
class param {
public:
	param() { tempo = 0; erroc = 0; errol = 0; erro = 0; dt = 0;}	/*Construtor da classe*/
	double tempo; /*Instante t da análise*/
	double erroc;	
	double errol;
	double erro;
	double dt;
};

/*Protótipos das funções utilizadas no programa*/

/*Esta função resolve um sistema de equações*/
int ResolverSistema(matriz sistema, matriz &outSistema);

/*Ao rodar essa função os nós do circuito a ser analisado são nomeados*/
int NomearNos(string nome, vector<string> &lista);

/*Ao rodar essa função o programa lê o netlist do circuito a ser analisado a partir do arquivo .NET de entrada*/
int ObterNetlist(string nomeArquino, netlist &netlist, vector<string> &lista, Dados_Analise &informacoes, Dados_NR &infoNetownRapson);

/*Ao rodar essa função o programa define as tensões e correntes a serem calculadas*/
int ConfigurarNetList(netlist &netList, vector<string> &lista);

/*Essa função preenche o sistema de equações a ser resolvido adicionando ao mesmo as estampas de cada componente do circuito sob análise*/
/*Em caso de componentes não lineares ou variantes no tempo, as estampas dos mesmos são modificadas em outro momento por outras funções ou métodos*/
int Estampar(netlist netList, matriz &sistema, size_t num_Val);

/*Ao rodar essa função o programa gera o arquivo .TAB de saída*/
int SalvarResultados(ofstream &arquivo,vector<string> &lista ,matriz sistema, param parametros, Dados_Analise informacao);

/*Esta função calcula o valor de uma fonte senoidal em cada instante de tempo*/
double CalcularSenoide(vector<string> seno, double tempo);

/*Esta função calcula o valor de uma fonte pulsante em cada instante de tempo*/
double CalcularPulsante(vector<string> pulso, double tempo, double passo);

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/