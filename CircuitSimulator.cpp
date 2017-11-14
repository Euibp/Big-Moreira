/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém o código prinipal do programa*/

/*Bibliotecas necessárias*/
#include "stdafx.h"


/*Arquivo de header*/
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	cout << "****************************************" << endl << "****Simulador de Circuitos Eletricos****" << endl
		 << "****************************************" << endl << endl << "Informe o nome do arquivo: ";

	/*Variáveis e Objetos*/
	string nome = argv[1];			/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*/
//	string nome;			/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*///
	string enter;
	netlist net_List;				/*Netlist do circuito a ser analisado*/
	vector<string> lista;			/*Vetor que armazena nomes das variáveis a serem calculadas. Por extensão, armazena os nomes do nós do circuito*/
	matriz sistema;					/*Sistema de equações, com as estampas dos componentes, num instante t de análise, a ser resolvido*/
	matriz sistemaResolvido;		/*Sistema de equações resolvido, para um instante t de análise*/
	ofstream arquivo;				/*Arquivo sendo manipulado*/
	param parametros;				/*Objeto que armazena as informações a serem impressas no arquivo .TAB de saída*/
	Dados_Analise dadosAnalise;		/*Objeto que contém instruções para uma análise no tempo*/
	Dados_NR dadosNR;				/*Objeto que contém informações de componentes não lineraes*/
	int erro;						/*Armazena o código de erro de convergência de componentes não lineares*/

//	cin >> nome;

	/*Quando o programa começou a rodar*/
	clock_t startClock = clock();

	cout << endl << "simulando..." << endl;

	/*O netlist do circuito é obtido*/
	/*Os nós do circuito são nomeados*/
	/*As tensões e correntes a serem calculadas são determinadas*/
	ObterNetlist((nome+".net"), net_List, lista, dadosAnalise, dadosNR);
	
	/*O sistema de equações a ser resolvido é estampado*/
	Estampar(net_List, sistema, lista.size());

	/*O programa calcula aqui em quantos instantes de tempo o circuito precisará ser analisado*/
	dadosAnalise.NumeroDeOperacoes();

	/*O arquivo que contém a netlist do circuito a ser analisado é fechado */
	arquivo.close();
	
	/*O arquivo de saída .TAB é criado e aberto*/
	arquivo.open(nome + "_Nosso.TAB");

	/*É inicializado um loop de análise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		dadosAnalise.CalcularComponentesTempo(net_List);					/*Os valores dos componentes variáveis no tempo são calculados para o instante t de análise*/
		dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido); /*A estampa dos componentes variáveis no tempo é atualizada*/				
		ResolverSistema(sistema, sistemaResolvido);							/*O sistema de equações é resolvido*/
		
		/*Verifica-se se houve convergência para os componentes não lineares*/
		/*Em caso positivo o programa é encerrado com ERRO_DE_ESTABILIZAÇÂO*/
		erro = dadosNR.CalcularNewtonRaphson(net_List, sistema, sistemaResolvido);
		if(erro == ERRO_DE_ESTABILIZACAO){
			cout << "A análise não converge." << endl;
			return(ERRO_MAIN_NAO_CONVERGENCIA);
		};
	
		SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de análise é impresso no .TAB*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;									/*O instante t de análise é atualizado*/
	}
	
	/*O arquivo de saída .TAB é fechado*/
	arquivo.close();

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;
//	cin >> enter;

	return(SUCESSO);
}