//#ifndef
	#include "stdafx.h"
	#include <iostream>
	#include <string>
	#include <fstream>
	#include <vector>
	#include <cmath>
	#include <sstream>
	#include <iomanip>

//#else
//#endif


using std::ifstream;
using std::ofstream;
using namespace std;

#define MAX_NOME 11

#define MAX_LINHA 80
#define MAX_COMPONENTE 50
#define MAX_NOS 50
#define TOLG 1e-9
#define PI atan(1)*4

#define RESISTOR_DE_GAMBIARRA 1e9


typedef vector<vector<double>> matriz;
typedef vector<vector<string>> infcomp;

enum error {
	SUCESSO,
	ARQUIVO_INEXISTENTE,
	ERRO_NUMERO_MAXIMO_ELEMENTOS,
	ELEMENTO_DESCONHECIDO,
	NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES,
	SISTEMA_SINGULAR,
	ERRO_ABERTURA_ARQUIVO
};

class Componente{
public:
	Componente() { nome = ""; valor = 0.0; no_A = 0; no_B = 0; no_C = 0; no_D = 0; j_x = 0; j_y = 0; jc_0 = 0; }
	string tipo;
	string nome; /*Nome do componente*/
	double valor; /*Valor do componente*/
	int no_A; /*Número do Nó A onde o componente está ligado*/
	int	no_B; /*Número do Nó B onde o componente está ligado*/
	int	no_C; /*Se for um AmpOp, número do Nó C onde o componente está ligado OU se for uma fonte controlada, número do nó C do ramo de controle*/
	int	no_D; /*Se for um AmpOp, número do Nó D onde o componente está ligado OU se for uma fonte controlada, número do nó D do ramo de controle*/
	int	j_x;
	int	j_y;

	//Auxiliares
	double param_N;
	double jc_0;
};


typedef vector<Componente> netlist;

class Dados_Analise {
public:
	Dados_Analise() { tipo_Analise = ".TRAN"; tempo_Atual = 0; tempo_Final = 1; passo = 1; metodo = "TRAP"; passos_Tabela = 1; Seila = "desconhecido"; }
	string tipo_Analise;
	double tempo_Atual;
	double tempo_Final;
	double passo;
	string metodo;
	int passos_Tabela;
	string Seila;

	size_t numero_De_Analises;
	

	// informações dos componentes que variam com o tempo/frequencia;
	infcomp comp_var;
	vector<size_t> posicao_var;
	//vector<double> variacao_tensao;


	int CalcularComponentesTempo(netlist &net_List);
	int AtualizarEstampa(netlist &net_List, matriz &sistema, matriz sistema_Anterior);
	int NumeroDeOperacoes();
};

class param {
public:
	param() { tempo = 0; erroc = 0; errol = 0; erro = 0; dt = 0;	}
	double tempo;
	double erroc;
	double errol;
	double erro;
	double dt;
};

//typedef infcomp compVarTempo;

int ResolverSistema(matriz sistema, matriz &outSistema);
int NomearNos(string nome, vector<string> &lista);

int ObterNetlist(string nomeArquino, netlist &netlist, vector<string> &lista, Dados_Analise &informacoes);

int ConfigurarNetList(netlist &netList, vector<string> &lista);

int Estampar(netlist netList, matriz &sistema, size_t num_Val);

int ExibirResultados(matriz sistema);

int SalvarResultados(ofstream &arquivo,vector<string> &lista ,matriz sistema, param parametros, Dados_Analise informacao);


double CalcularNewtonRapson(vector<string> paramCompNR, double valorAnterior);

//#########################################################################################################
//Funções de Calculo.
double CalcularSenoide(vector<string> seno, double tempo);
double CalcularPulsante(vector<string> pulso, double tempo, double passo);


