/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém o código prinipal do programa*/

/*Bibliotecas necessárias*/
#include "stdafx.h"

/*Arquivo de header*/
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	/*Impressão de algumas informações*/
	cout << "****************************************" << endl << "****Simulador de Circuitos Eletricos****" << endl
		 << "****************************************" << endl;

	/*Variáveis e Objetos*/
	string nome = argv[1];				/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*/
	netlist net_List;					/*Netlist do circuito a ser analisado*/
	vector<string> lista;				/*Vetor que armazena nomes das variáveis a serem calculadas. Por extensão, armazena os nomes do nós do circuito*/
	matriz sistema;						/*Sistema de equações, com as estampas dos componentes, num instante t de análise, a ser resolvido*/
	matriz sistemaResolvido;			/*Sistema de equações resolvido, para um instante t de análise*/
	ofstream arquivo;					/*Arquivo sendo manipulado*/
	param parametros;					/*Objeto que armazena as informações a serem impressas no arquivo .TAB de saída*/
	Dados_Analise dadosAnalise;			/*Objeto que contém instruções para uma análise no tempo*/
	Dados_NR dadosNR;					/*Objeto que contém informações de componentes não lineraes*/
	int erroCalcularNewtonRaphson;		/*Armazena o código de erro de CalcularNewtonRaphson*/
	int erroObterNetlist;				/*Armazena o código de erro de ObterNetList*/
	int erroEstampar;					/*Armazena o código de erro de Estampar*/
	int erroResolverSistema;			/*Armazena o código de erro de ResolverSistema*/
	int erroNumeroDeOperacoes;			/*Armazena o código de erro de método de Methods_Time_Analize*/
	int erroCalcularComponentesTempo;	/*Armazena o código de erro de método de Methods_Time_Analize*/
	int erroAtualizarEstampa;			/*Armazena o código de erro de método de Methods_Time_Analize*/

	/*Quando o programa começou a rodar*/
	clock_t startClock = clock();

	/*O netlist do circuito é obtido*/
	/*Os nós do circuito são nomeados*/
	/*As tensões e correntes a serem calculadas são determinadas*/
	erroObterNetlist = ObterNetlist((nome), net_List, lista, dadosAnalise, dadosNR);
	if (erroObterNetlist != SUCESSO){
		if (erroObterNetlist == ERRO_ARQUIVO_INEXISTENTE){
			cout << "O Netlist informado nao existe." << endl;
			return(ERRO_ARQUIVO_INEXISTENTE);
		};
		if (erroObterNetlist == ERRO_NUMERO_MAXIMO_ELEMENTOS){
			cout << "O Netlist possui mais elementos que o permitido." << endl;
			return(ERRO_NUMERO_MAXIMO_ELEMENTOS);
		};
		if (erroObterNetlist == ERRO_ELEMENTO_DESCONHECIDO){
			cout << "O Netlist possui um ou mais elementos desconhecidos."<< endl;
			return(ERRO_ELEMENTO_DESCONHECIDO);
		};
		if (erroObterNetlist == ERRO_NUMERO_MAXIMO_NOS){
			cout << "O circuito a ser analisado possui um numero muito grande de nos."<< endl;
			return(ERRO_NUMERO_MAXIMO_NOS);
		};
		if (erroObterNetlist == ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES){
			cout << "O circuito a ser analisado requer o calculo de um numero grande de correntes." << endl;
			return(ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
		};
	};

	/*O sistema de equações a ser resolvido é estampado*/
	erroEstampar = Estampar(net_List, sistema, lista.size());
	if (erroEstampar == ERRO_ESTAMPAR){
		cout << "Nao foi possivel construir um sistema de equacoes para resolver." << endl;
		return(ERRO_ESTAMPAR);
	};

	/*O programa calcula aqui em quantos instantes de tempo o circuito precisará ser analisado*/
	erroNumeroDeOperacoes = dadosAnalise.NumeroDeOperacoes();
	if (erroNumeroDeOperacoes != SUCESSO){
		cout << "Nao foi possivel calcular o numero de operacoes a serem realizadas." << endl;
		return(ERRO_NUMERO_DE_OPERACOES);
	};


	/*O arquivo que contém a netlist do circuito a ser analisado é fechado */
	arquivo.close();
	
	/*O arquivo de saída .TAB é criado e aberto*/
	arquivo.open(nome + ".TAB");

	/*É inicializado um loop de análise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		/*Os valores dos componentes variáveis no tempo são calculados para o instante t de análise*/
		erroCalcularComponentesTempo = dadosAnalise.CalcularComponentesTempo(net_List);
		if (erroCalcularComponentesTempo != SUCESSO){
			cout << "Nao foi possivel calcular os valores dos componentes variantes no tempo." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_CALCULO_COMP_TEMPO);
		};

		/*A estampa dos componentes variáveis no tempo é atualizada*/
		erroAtualizarEstampa = dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido);
		if (erroAtualizarEstampa != SUCESSO){
			cout << "Nao foi possivel realizar a analise no tempo." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_ATUALIZAR_ESTAMPA);
		};

		/*O sistema de equações é resolvido*/
		erroResolverSistema = ResolverSistema(sistema, sistemaResolvido);	
		if (erroResolverSistema == ERRO_SISTEMA_SINGULAR){
			cout << "Foi encontrado um sistema singular." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_SISTEMA_SINGULAR);
		};

		/*Verifica-se se houve convergência para os componentes não lineares*/
		/*Em caso negativo o programa é encerrado com ERRO_DE_ESTABILIZAÇÂO*/
		erroCalcularNewtonRaphson = dadosNR.CalcularNewtonRaphson(net_List, sistema, sistemaResolvido);
		if(erroCalcularNewtonRaphson == ERRO_DE_ESTABILIZACAO){
			cout << "A analise nao converge." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_MAIN_NAO_CONVERGENCIA);
		};

		/*São impressos no arquivo .TAB de saída apenas os resultados das análises em instantes t = a*passo, com a inteiro positivo*/
		if ((indice) % dadosAnalise.passos_Tabela == 0 || indice == 0)
			SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de análise é impresso no .TAB*/
		
		/*O instante t de análise é atualizado*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;										
	}
	
	/*O arquivo de saída .TAB é fechado*/
	arquivo.close();
	cout << "O circuito foi analisado com sucesso."<< endl;

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

	/*A análise do circuito foi concluída sem problemas*/
	return(SUCESSO);
}