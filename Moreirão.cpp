// Moreirão.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Header_CircuitSimulator.h"


int main(int argc, char** argv)
{
	string nome = argv[1];
	clock_t startClock = clock();

	int erro;

	netlist net_List;
	vector<string> lista;
	matriz sistema, sistemaResolvido;
	ofstream arquivo;
	param parametros;
	Dados_Analise dadosAnalise;
	Dados_NR dadosNR;

	erro = ObterNetlist((nome+".net"), net_List, lista, dadosAnalise, dadosNR);
	Estampar(net_List, sistema, lista.size());

	//for (size_t row = 1; row < sistema.size(); row++) {
	//	for (size_t col = 1; col < sistema[row].size(); col++) {
	//		cout << setw(4) << setprecision(2) << sistema[row][col] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << endl;


	dadosAnalise.NumeroDeOperacoes();
	arquivo.close();
	arquivo.open(nome + ".TAB");
	//arquivo << "[";
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++) {
		dadosAnalise.CalcularComponentesTempo(net_List);
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido);
		
		//cout << " INDICE AQUI  :" << indice <<endl;
		//if (indice < 20) {
		//	cout << "Não resolvido" << endl;
		//	for (size_t row = 0; row < sistema.size(); row++) {
		//		for (size_t col = 0; col < sistema[row].size(); col++) {
		//			cout << setw(4) << setprecision(2) << sistema[row][col] << " ";
		//		}
		//		cout << endl;
		//	}
		//	cout << endl;
		//}
		
		ResolverSistema(sistema, sistemaResolvido);
	
		//if (indice < 20) {
		//	cout << "Resolvido" << endl;
		//	for (size_t row = 1; row < sistemaResolvido.size(); row++) {
		//		for (size_t col = 1; col < sistemaResolvido[row].size(); col++) {
		//			cout << setw(4) << setprecision(2) << sistemaResolvido[row][col] << " ";
		//		}
		//		cout << endl;
		//	}
		//	cout << endl;
		//}
		//arquivo << sistema[1][sistema.size()] << ";" ;
		dadosNR.CalcularNewtonRapson(net_List, sistema, sistemaResolvido);
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);

		dadosAnalise.tempo_Atual += dadosAnalise.passo;

	}
	//arquivo << "]";
	arquivo.close();

	clock_t endClock = clock();
	cout << "Compiler Time: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

    return 0;
}