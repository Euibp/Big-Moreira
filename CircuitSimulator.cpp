/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém o código prinipal do programa*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	/*Quando o programa começou a rodar*/
	clock_t startClock = clock();
	cout << "Processando... " << endl;

	/*Variáveis e Objetos*/
	string nome = argv[1];			/*Nome do arquivo .NET de entrada*/
	netlist net_List;				/*Netlist do circuito a ser analisado*/
	vector<string> lista;
	matriz sistema;					/*Sistema de equações, com as estampas dos componentes, num instante t de análise*/
	matriz sistemaResolvido;		/*Sistema de equações resolvido*/
	ofstream arquivo;				/*Arquivo sendo manipulado*/
	param parametros;				/*Objeto que armazena as informações a serem impressas no arquivo .TAB de saída*/
	Dados_Analise dadosAnalise;		/*Objeto que contém instruções para uma análise no tempo*/
	Dados_NR dadosNR;				/*Objeto que contém informações de componentes não lineraes*/

	/*O netlist do circuito é obtido*/
	/*Os nós do circuito são nomeados*/
	/*As tensões e correntes a serem calculadas são determinadas*/
	ObterNetlist((nome+".net"), net_List, lista, dadosAnalise, dadosNR);
	
	/*O sistema de equações a ser resolvido é estampado*/
	Estampar(net_List, sistema, lista.size());

	/**/
	dadosAnalise.NumeroDeOperacoes();
	
	/**/
	arquivo.close();
	
	/*O arquivo de saída .TAB é criado e aberto*/
	arquivo.open(nome + ".TAB");

	/*É inicializado um loop de análise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		dadosAnalise.CalcularComponentesTempo(net_List);					/*Os valores dos componentes variáveis no tempo são calculados para o instante t de análise*/
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido); /*A estampa dos componentes variáveis no tempo é atualizada*/		
		ResolverSistema(sistema, sistemaResolvido);							/*O sistema de equações é resolvido*/
		dadosNR.CalcularNewtonRapson(net_List, sistema, sistemaResolvido);	/*A próxima aproximação do método de Newton-Raphson é calculada*/
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de análise é impresso no .TAB*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;									/*O instante t de análise é atualizado*/
	}
	
	/*O arquivo de saída .TAB é fechado*/
	arquivo.close();

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

	return(SUCESSO);
}