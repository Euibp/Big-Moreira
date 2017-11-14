/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m o c�digo prinipal do programa*/

/*Bibliotecas necess�rias*/
#include "stdafx.h"


/*Arquivo de header*/
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	cout << "****************************************" << endl << "****Simulador de Circuitos Eletricos****" << endl
		 << "****************************************" << endl << endl << "Informe o nome do arquivo: ";

	/*Vari�veis e Objetos*/
	string nome = argv[1];			/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*/
//	string nome;			/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*///
	string enter;
	netlist net_List;				/*Netlist do circuito a ser analisado*/
	vector<string> lista;			/*Vetor que armazena nomes das vari�veis a serem calculadas. Por extens�o, armazena os nomes do n�s do circuito*/
	matriz sistema;					/*Sistema de equa��es, com as estampas dos componentes, num instante t de an�lise, a ser resolvido*/
	matriz sistemaResolvido;		/*Sistema de equa��es resolvido, para um instante t de an�lise*/
	ofstream arquivo;				/*Arquivo sendo manipulado*/
	param parametros;				/*Objeto que armazena as informa��es a serem impressas no arquivo .TAB de sa�da*/
	Dados_Analise dadosAnalise;		/*Objeto que cont�m instru��es para uma an�lise no tempo*/
	Dados_NR dadosNR;				/*Objeto que cont�m informa��es de componentes n�o lineraes*/
	int erro;						/*Armazena o c�digo de erro de converg�ncia de componentes n�o lineares*/

//	cin >> nome;

	/*Quando o programa come�ou a rodar*/
	clock_t startClock = clock();

	cout << endl << "simulando..." << endl;

	/*O netlist do circuito � obtido*/
	/*Os n�s do circuito s�o nomeados*/
	/*As tens�es e correntes a serem calculadas s�o determinadas*/
	ObterNetlist((nome+".net"), net_List, lista, dadosAnalise, dadosNR);
	
	/*O sistema de equa��es a ser resolvido � estampado*/
	Estampar(net_List, sistema, lista.size());

	/*O programa calcula aqui em quantos instantes de tempo o circuito precisar� ser analisado*/
	dadosAnalise.NumeroDeOperacoes();

	/*O arquivo que cont�m a netlist do circuito a ser analisado � fechado */
	arquivo.close();
	
	/*O arquivo de sa�da .TAB � criado e aberto*/
	arquivo.open(nome + "_Nosso.TAB");

	/*� inicializado um loop de an�lise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		dadosAnalise.CalcularComponentesTempo(net_List);					/*Os valores dos componentes vari�veis no tempo s�o calculados para o instante t de an�lise*/
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido); /*A estampa dos componentes vari�veis no tempo � atualizada*/				
		ResolverSistema(sistema, sistemaResolvido);							/*O sistema de equa��es � resolvido*/
		
		/*Verifica-se se houve converg�ncia para os componentes n�o lineares*/
		/*Em caso positivo o programa � encerrado com ERRO_DE_ESTABILIZA��O*/
		erro = dadosNR.CalcularNewtonRaphson(net_List, sistema, sistemaResolvido);
		if(erro == ERRO_DE_ESTABILIZACAO){
			cout << "A an�lise n�o converge." << endl;
			return(ERRO_MAIN_NAO_CONVERGENCIA);
		};
	
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de an�lise � impresso no .TAB*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;									/*O instante t de an�lise � atualizado*/
	}
	
	/*O arquivo de sa�da .TAB � fechado*/
	arquivo.close();

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;
//	cin >> enter;

	return(SUCESSO);
}