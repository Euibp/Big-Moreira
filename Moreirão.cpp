// Moreirão.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Header_CircuitSimulator.h"


int main()
{
	netlist net_List;
	vector<string> lista;
	cout << ObterNetlist("teste.net", net_List, lista);
    return 0;
}