// Moreirão.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Header_CircuitSimulator.h"


int main(int argc, char** argv)
{
	string nome = argv[1];
	cout << nome;
	clock_t startClock = clock();

	netlist net_List;
	vector<string> lista;
	matriz sistema, sistemaResolvido;
	ofstream arquivo;
	param parametros;
	Dados_Analise dadosAnalise;
	ObterNetlist((nome+".net"), net_List, lista, dadosAnalise);
	Estampar(net_List, sistema, lista.size());

	dadosAnalise.NumeroDeOperacoes();
	arquivo.close();
	arquivo.open(nome + ".TAB");
	//arquivo << "[";
	for (size_t indice = 0; indice < dadosAnalise.numero_De_Analises; indice++) {
		dadosAnalise.CalcularComponentesTempo(net_List);
		dadosAnalise.AtualizarEstampa(net_List, sistema);
		ResolverSistema(sistema, sistemaResolvido);
		
		//arquivo << sistema[1][sistema.size()] << ";" ;

		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);

		dadosAnalise.tempo_Atual += dadosAnalise.passo;
	}
	//arquivo << "]";
	arquivo.close();

	clock_t endClock = clock();
	cout << "Compiler Time: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

	cout << "endl";
    return 0;
}