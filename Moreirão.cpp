// Moreirão.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Header_CircuitSimulator.h"


int main(int argc, char** argv)
{
	string nome = argv[1];
	clock_t startClock = clock();

	netlist net_List;
	vector<string> lista;
	matriz sistema, sistemaResolvido;
	ofstream arquivo;
	param parametros;
	Dados_Analise dadosAnalise;
	cout << ObterNetlist((nome+".net"), net_List, lista, dadosAnalise);
	Estampar(net_List, sistema, lista.size());

	for (size_t row = 1; row < sistema.size(); row++) {
		for (size_t col = 1; col < sistema[row].size(); col++) {
			cout << setw(4) << setprecision(2) << sistema[row][col] << " ";
		}
		cout << endl;
	}
	cout << endl;


	dadosAnalise.NumeroDeOperacoes();
	arquivo.close();
	arquivo.open(nome + ".TAB");
	//arquivo << "[";
	for (size_t indice = 0; indice < dadosAnalise.numero_De_Analises; indice++) {
		dadosAnalise.CalcularComponentesTempo(net_List);
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido);

		//if (indice < 8) {
		//	cout << "Não resolvido" << endl;
		//	for (size_t row = 1; row < sistema.size(); row++) {
		//		for (size_t col = 1; col < sistema[row].size(); col++) {
		//			cout << setw(4) << setprecision(2) << sistema[row][col] << " ";
		//		}
		//		cout << endl;
		//	}
		//	cout << endl;
		//}
		//cout << " INDICE AQUI  :" << indice <<endl;
		ResolverSistema(sistema, sistemaResolvido);
	
		//if (indice < 8) {
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
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);

		dadosAnalise.tempo_Atual += dadosAnalise.passo;

	}
	//arquivo << "]";
	arquivo.close();

	clock_t endClock = clock();
	cout << "Compiler Time: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

    return 0;
}