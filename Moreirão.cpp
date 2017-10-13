// Moreirão.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Header_CircuitSimulator.h"


int main()
{
	netlist net_List;
	vector<string> lista;
	matriz sistema;
	ofstream arquivo;
	param parametros;

	ObterNetlist("teste.net", net_List, lista);
	Estampar(net_List, sistema, lista.size());
	ResolverSistema(sistema);

	arquivo.open("teste.TAB");

	SalvarResultados(arquivo, lista, sistema, parametros);
	arquivo.close();

    return 0;
}