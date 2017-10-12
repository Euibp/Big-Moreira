//#ifndef
	#include "stdafx.h"
	#include <iostream>
	#include <string>
	#include <fstream>
	#include <vector>
	#include <sstream>

//#else
//#endif


using std::ifstream;
using std::ofstream;
using namespace std;

#define MAX_NOME 11

#define MAX_LINHA 80
#define MAX_COMPONENTE 50
#define MAX_NOS 50


class Componente{
public:
	Componente() { nome = ""; valor = 0.0; no_A = 0; no_B = 0; no_C = 0; no_D = 0; j_x = 0; j_y = 0; }
	string tipo;
	string nome; /*Nome do componente*/
	double valor; /*Valor do componente*/
	int no_A; /*Número do Nó A onde o componente está ligado*/
	int	no_B; /*Número do Nó B onde o componente está ligado*/
	int	no_C; /*Se for um AmpOp, número do Nó C onde o componente está ligado OU se for uma fonte controlada, número do nó C do ramo de controle*/
	int	no_D; /*Se for um AmpOp, número do Nó D onde o componente está ligado OU se for uma fonte controlada, número do nó D do ramo de controle*/
	int	j_x;
	int	j_y;
};

typedef vector<Componente> netlist;
typedef vector<vector<int>> matriz;

int ResolverSistema(matriz &sistema);
int NomearNos(string nome, vector<string> &lista);
int ObterNetlist(string nomeArquino, netlist &netlist, vector<string> &lista);
int ConfigurarNetList(netlist &netList, vector<string> &lista);

int Estampar(netlist netList, matriz &sistema, vector<string> lista);

int SalvarResultados(ofstream &arquivo, matriz sistema);
int ExibirResultados(matriz sistema);



