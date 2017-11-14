/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m inclus�o de bibliotecas e namespaces, defini��o de macros, defini��o de c�digos de erro,
  declara��o das classes e das fun��es utilizadas, defini��o de tipos*/

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
#define MAX_NOME 11					/*Tamanho m�ximo que o nome de um arquivo .NET pode ter*/
#define MAX_LINHA 80				/*Tamanho m�ximo que uma linha de um arquivo .NET pode ter*/
#define MAX_COMPONENTE 100			/*Tamanho m�ximo de componentes em um arquivo .NET*/
#define MAX_NOS 100					/*Tamanho m�ximo de n�s que um circuito, definido em um arquivo .NET, pode ter*/
#define MAX_CORRENTES_EXTRAS 50		/*M�ximo n�mero de correntes que o programa pode adicionar a um circuito para calcular correntes em ramos*/
#define TOLG 1e-9					/*Toler�ncia para determinante, a fim de detectar se um sistema � singular */
#define PI atan(1)*4				/*Defini��o do cl�ssico n�mero PI*/
#define RESISTOR_DE_GAMBIARRA 1e9	/*Resistor utilizado no c�lculo do ponto de opera��o de circuitos com capacitores e indutores*/

/*C�digos de Erro que o Programa Retorna*/
enum error 
{
	SUCESSO,									/*Caso o programa execute por completo, sem erros*/

	/*Erros ao executar CircuitSimulator.cpp*/
	ERRO_MAIN_NAO_CONVERGENCIA,					/*Caso em que o programa � encerrado por um componente n�o linear n�o ter convergido*/
	
	/*Erros ao executar Functions_CircuitSimulator.cpp*/
	ERRO_ARQUIVO_INEXISTENTE,					/*Caso um arquivo .NET, fornecido como entrada para o programa, n�o exista*/
	ERRO_NUMERO_MAXIMO_ELEMENTOS,				/*Caso o arquivo .NET fornecido como entrada tenha mais de 100 elementos*/
	ERRO_ELEMENTO_DESCONHECIDO,					/*Caso o arquivo .NET fornecido como entrada possua um elemento desconhecido*/
	ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES,	/*Caso o n�mero de correntes adicionadas ao circuito pelo programa exceda um valor m�ximo*/
	ERRO_ESTAMPAR,								/*Caso tenha-se tentado estampar um componente que n�o existe*/
	ERRO_NUMERO_MAXIMO_NOS,						/*Caso o arquivo .NET fornecido como entrada descreva um circuito com mais de 100 n�s*/
	ERRO_SISTEMA_SINGULAR,							/*Caso, ao resolver um sistema de equa��es, o programa se deparar com um sistema singular*/
	
	/*Erros ao executar Methods_NewtonRaphson_Algorithm*/
	ESTABILIZOU,								/*Caso uma an�lise com elementos n�o-lineares tenha estabilizado*/
	ERRO_DE_ESTABILIZACAO,						/*Caso uma an�lise com elementos n�o-lineares n�o tenha estabilizado*/
	ERRO_ESTAMPAR_NAO_LINEAR,					/*Caso n�o exista chaves ou resistores n�o lineares e uma atualiza��o de estampa para elementos n�o lineares for pedida*/
	ERRO_COMPONENTE_NAO_CALCULADO,				/*Caso n�o exista chaves ou resistores n�o lineares e um calculo desses componentes foi solicitado*/

	/*Erros ao executar Methods_Time_Analize*/
};

/*Classe que cont�m os atributos de um componente gen�rico que um circuito pode ter*/
class Componente{
public:
	Componente() { nome = ""; valor = 0.0; no_A = 0; no_B = 0; no_C = 0; no_D = 0; j_x = 0; j_y = 0; jc_0 = 0; valor_novo = 0; Io = 0; gmin = 0; }	/*Construtor da classe*/
	string tipo;			/*Qual componente �? R? C? L?*/
	string nome;			/*Nome fantasia do componente. Ex: R0100*/
	double valor;			/*Valor do componente*/

	int no_A;				/*N�mero do N� A onde o componente est� ligado*/
	int	no_B;				/*N�mero do N� B onde o componente est� ligado*/
	int	no_C;				/*Se for um AmpOp, n�mero do N� C onde o componente est� ligado OU se for uma fonte controlada(chave), n�mero do n� C do ramo de controle*/
	int	no_D;				/*Se for um AmpOp, n�mero do N� D onde o componente est� ligado OU se for uma fonte controlada(chave), n�mero do n� D do ramo de controle*/

	int	j_x;				/*N�mero da coluna/linha de uma matriz correspondente � uma corrente X*/
	int	j_y;				/*N�mero da coluna/linha de uma matriz correspondente � uma corrente Y*/

	double valor_novo;		/*Vari�vel auxiliar para armezenar um valor*/
	double Io;				/*Vari�vel auxiliar para armezenar um valor inicial de corrente em Newton-Raphson*/						
	double jc_0;			/*Vari�vel auxiliar para armezenar um valor de corrente em capacitores*/

	double gmin;			/*Valor de condut�ncia utilizada no processo de Gmin stepping*/
};

/*Tipos de Vari�vel Utilizados*/
typedef vector<vector<double>> matriz;	/*Uma matriz onde todos os valores podem possuir casas decimais*/
typedef vector<vector<string>> infcomp; /*Uma matriz onde todos os valores s�o strings*/
typedef vector<Componente> netlist;		/*Um vetor de elementos da classe componente*/

/*Classe que cont�m instru��es para uma an�lise no tempo*/
class Dados_Analise {
public:
	Dados_Analise() { tipo_Analise = ".TRAN"; tempo_Atual = 0; tempo_Final = 1; passo = 1; metodo = "TRAP"; passos_Tabela = 1; } /*Construtor da classe*/
	string tipo_Analise;			/*Armazena a informa��o .TRAN, que diz ao programa que uma an�lise no tempo deve ser feita*/
	double tempo_Atual;				/*Tempo t em que a an�lise est� sendo realizada*/
	double tempo_Final;				/*Tempo de dura��o da an�lise*/
	double passo;					/*Tempo entre um c�lculo e outro durante a an�lise. Ex: O programa calcula uma tens�o para t e para t+passo*/
	string metodo;					/*M�todo da an�lise. Ex: TRAP, BE, FE*/
	int passos_Tabela;
	size_t numero_De_Analises;		/*Quantidade de an�lises no tempo que devem ser feitas. tempo total / passo*/
	infcomp comp_var;				/*Matriz que cont�m informa��es de um componente vari�vel no tempo. Ex: Fontes senoidais*/
	vector<size_t> posicao_var;
	
	/*M�todos que pertencem � classe*/
	int CalcularComponentesTempo(netlist &net_List);									/*Calcula o valor das fontes pulsantes e senoidais em cada instante de tempo*/
	int AtualizarEstampa(netlist &net_List, matriz &sistema, matriz sistema_Anterior);	/*Atualiza a estampa dos componentes vari�veis no tempo durante a an�lise*/
	int NumeroDeOperacoes();															/*Calcula quantas an�lises no tempo devem ser realizadas*/
};

/*Classe que cont�m informa��es de componentes n�o lineraes*/
class Dados_NR {
public:
	infcomp comp_var;				/*Matriz que cont�m os par�metros de elementos n�o lineares*/
	vector<size_t> posicao_var;

	/*M�todos que pertencem � classe*/
	int CalcularNewtonRaphson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior);			/*Calcula a pr�xima aproxima��o quando no M�todo de Newton-Raphson*/
	int EstampaNR(matriz &sistema, netlist &net_list, char tipo, size_t indice, double novo_valor); /*Atualiza a estampa dos componentes n�o lineares durante a an�lise*/
	double CalcularValorNR(vector<string> paramNR, double valorAnterior, double &I0);				/*Calcula o pr�ximo valor de um componente n�o linear*/
	int GminStep(matriz &sistema, netlist &net_List, char tipo, size_t indice, bool convergencia, double &fator);
	size_t InteracaoNR(matriz &sistema, netlist &net_List, matriz &sistema_Anterior, vector<bool> &verifica_Convergencia);
};

/*Classe que armazena as informa��es a serem impressas no arquivo .TAB de sa�da*/
class param {
public:
	param() { tempo = 0; erroc = 0; errol = 0; erro = 0; dt = 0;}	/*Construtor da classe*/
	double tempo; /*Instante t da an�lise*/
	double erroc;	
	double errol;
	double erro;
	double dt;
};

/*Prot�tipos das fun��es utilizadas no programa*/

/*Esta fun��o resolve um sistema de equa��es*/
int ResolverSistema(matriz sistema, matriz &outSistema);

/*Ao rodar essa fun��o os n�s do circuito a ser analisado s�o nomeados*/
int NomearNos(string nome, vector<string> &lista);

/*Ao rodar essa fun��o o programa l� o netlist do circuito a ser analisado a partir do arquivo .NET de entrada*/
int ObterNetlist(string nomeArquino, netlist &netlist, vector<string> &lista, Dados_Analise &informacoes, Dados_NR &infoNetownRapson);

/*Ao rodar essa fun��o o programa define as tens�es e correntes a serem calculadas*/
int ConfigurarNetList(netlist &netList, vector<string> &lista);

/*Essa fun��o preenche o sistema de equa��es a ser resolvido adicionando ao mesmo as estampas de cada componente do circuito sob an�lise*/
/*Em caso de componentes n�o lineares ou variantes no tempo, as estampas dos mesmos s�o modificadas em outro momento por outras fun��es ou m�todos*/
int Estampar(netlist netList, matriz &sistema, size_t num_Val);

/*Ao rodar essa fun��o o programa gera o arquivo .TAB de sa�da*/
int SalvarResultados(ofstream &arquivo,vector<string> &lista ,matriz sistema, param parametros, Dados_Analise informacao);

/*Esta fun��o calcula o valor de uma fonte senoidal em cada instante de tempo*/
double CalcularSenoide(vector<string> seno, double tempo);

/*Esta fun��o calcula o valor de uma fonte pulsante em cada instante de tempo*/
double CalcularPulsante(vector<string> pulso, double tempo, double passo);

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/