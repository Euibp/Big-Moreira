/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m o c�digo prinipal do programa*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	/*Quando o programa come�ou a rodar*/
	clock_t startClock = clock();
	cout << "Processando... " << endl;

	/*Vari�veis e Objetos*/
	string nome = argv[1];			/*Nome do arquivo .NET de entrada*/
	netlist net_List;				/*Netlist do circuito a ser analisado*/
	vector<string> lista;
	matriz sistema;					/*Sistema de equa��es, com as estampas dos componentes, num instante t de an�lise*/
	matriz sistemaResolvido;		/*Sistema de equa��es resolvido*/
	ofstream arquivo;				/*Arquivo sendo manipulado*/
	param parametros;				/*Objeto que armazena as informa��es a serem impressas no arquivo .TAB de sa�da*/
	Dados_Analise dadosAnalise;		/*Objeto que cont�m instru��es para uma an�lise no tempo*/
	Dados_NR dadosNR;				/*Objeto que cont�m informa��es de componentes n�o lineraes*/

	/*O netlist do circuito � obtido*/
	/*Os n�s do circuito s�o nomeados*/
	/*As tens�es e correntes a serem calculadas s�o determinadas*/
	ObterNetlist((nome+".net"), net_List, lista, dadosAnalise, dadosNR);
	
	/*O sistema de equa��es a ser resolvido � estampado*/
	Estampar(net_List, sistema, lista.size());

	/**/
	dadosAnalise.NumeroDeOperacoes();
	
	/**/
	arquivo.close();
	
	/*O arquivo de sa�da .TAB � criado e aberto*/
	arquivo.open(nome + ".TAB");

	/*� inicializado um loop de an�lise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		dadosAnalise.CalcularComponentesTempo(net_List);					/*Os valores dos componentes vari�veis no tempo s�o calculados para o instante t de an�lise*/
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido); /*A estampa dos componentes vari�veis no tempo � atualizada*/		
		ResolverSistema(sistema, sistemaResolvido);							/*O sistema de equa��es � resolvido*/
		dadosNR.CalcularNewtonRapson(net_List, sistema, sistemaResolvido);	/*A pr�xima aproxima��o do m�todo de Newton-Raphson � calculada*/
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de an�lise � impresso no .TAB*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;									/*O instante t de an�lise � atualizado*/
	}
	
	/*O arquivo de sa�da .TAB � fechado*/
	arquivo.close();

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

	return(SUCESSO);
}