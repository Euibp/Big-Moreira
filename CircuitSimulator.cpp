/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m o c�digo prinipal do programa*/

/*Bibliotecas necess�rias*/
#include "stdafx.h"

/*Arquivo de header*/
#include "Header_CircuitSimulator.h"

int main(int argc, char** argv)
{
	/*Impress�o de algumas informa��es*/
	cout << "****************************************" << endl << "****Simulador de Circuitos Eletricos****" << endl
		 << "****************************************" << endl;

	/*Vari�veis e Objetos*/
	string nome = argv[1];				/*Nome do arquivo .NET de entrada. O nome do arquivo contendo o netlist deve ser inserido como argumento ao rodar o programa*/
	netlist net_List;					/*Netlist do circuito a ser analisado*/
	vector<string> lista;				/*Vetor que armazena nomes das vari�veis a serem calculadas. Por extens�o, armazena os nomes do n�s do circuito*/
	matriz sistema;						/*Sistema de equa��es, com as estampas dos componentes, num instante t de an�lise, a ser resolvido*/
	matriz sistemaResolvido;			/*Sistema de equa��es resolvido, para um instante t de an�lise*/
	ofstream arquivo;					/*Arquivo sendo manipulado*/
	param parametros;					/*Objeto que armazena as informa��es a serem impressas no arquivo .TAB de sa�da*/
	Dados_Analise dadosAnalise;			/*Objeto que cont�m instru��es para uma an�lise no tempo*/
	Dados_NR dadosNR;					/*Objeto que cont�m informa��es de componentes n�o lineraes*/
	int erroCalcularNewtonRaphson;		/*Armazena o c�digo de erro de CalcularNewtonRaphson*/
	int erroObterNetlist;				/*Armazena o c�digo de erro de ObterNetList*/
	int erroEstampar;					/*Armazena o c�digo de erro de Estampar*/
	int erroResolverSistema;			/*Armazena o c�digo de erro de ResolverSistema*/
	int erroNumeroDeOperacoes;			/*Armazena o c�digo de erro de m�todo de Methods_Time_Analize*/
	int erroCalcularComponentesTempo;	/*Armazena o c�digo de erro de m�todo de Methods_Time_Analize*/
	int erroAtualizarEstampa;			/*Armazena o c�digo de erro de m�todo de Methods_Time_Analize*/

	/*Quando o programa come�ou a rodar*/
	clock_t startClock = clock();

	/*O netlist do circuito � obtido*/
	/*Os n�s do circuito s�o nomeados*/
	/*As tens�es e correntes a serem calculadas s�o determinadas*/
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

	/*O sistema de equa��es a ser resolvido � estampado*/
	erroEstampar = Estampar(net_List, sistema, lista.size());
	if (erroEstampar == ERRO_ESTAMPAR){
		cout << "Nao foi possivel construir um sistema de equacoes para resolver." << endl;
		return(ERRO_ESTAMPAR);
	};

	/*O programa calcula aqui em quantos instantes de tempo o circuito precisar� ser analisado*/
	erroNumeroDeOperacoes = dadosAnalise.NumeroDeOperacoes();
	if (erroNumeroDeOperacoes != SUCESSO){
		cout << "Nao foi possivel calcular o numero de operacoes a serem realizadas." << endl;
		return(ERRO_NUMERO_DE_OPERACOES);
	};


	/*O arquivo que cont�m a netlist do circuito a ser analisado � fechado */
	arquivo.close();
	
	/*O arquivo de sa�da .TAB � criado e aberto*/
	arquivo.open(nome + ".TAB");

	/*� inicializado um loop de an�lise no tempo*/
	for (size_t indice = 0; indice <= dadosAnalise.numero_De_Analises; indice++)
	{
		/*Os valores dos componentes vari�veis no tempo s�o calculados para o instante t de an�lise*/
		erroCalcularComponentesTempo = dadosAnalise.CalcularComponentesTempo(net_List);
		if (erroCalcularComponentesTempo != SUCESSO){
			cout << "Nao foi possivel calcular os valores dos componentes variantes no tempo." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_CALCULO_COMP_TEMPO);
		};

		/*A estampa dos componentes vari�veis no tempo � atualizada*/
		erroAtualizarEstampa = dadosAnalise.AtualizarEstampa(net_List, sistema, sistemaResolvido);
		if (erroAtualizarEstampa != SUCESSO){
			cout << "Nao foi possivel realizar a analise no tempo." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_ATUALIZAR_ESTAMPA);
		};

		/*O sistema de equa��es � resolvido*/
		erroResolverSistema = ResolverSistema(sistema, sistemaResolvido);	
		if (erroResolverSistema == ERRO_SISTEMA_SINGULAR){
			cout << "Foi encontrado um sistema singular." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_SISTEMA_SINGULAR);
		};

		/*Verifica-se se houve converg�ncia para os componentes n�o lineares*/
		/*Em caso negativo o programa � encerrado com ERRO_DE_ESTABILIZA��O*/
		erroCalcularNewtonRaphson = dadosNR.CalcularNewtonRaphson(net_List, sistema, sistemaResolvido);
		if(erroCalcularNewtonRaphson == ERRO_DE_ESTABILIZACAO){
			cout << "A analise nao converge." << "    Erro no Tempo :" << dadosAnalise.tempo_Atual << endl;
			return(ERRO_MAIN_NAO_CONVERGENCIA);
		};

		/*S�o impressos no arquivo .TAB de sa�da apenas os resultados das an�lises em instantes t = a*passo, com a inteiro positivo*/
		if ((indice) % dadosAnalise.passos_Tabela == 0 || indice == 0)
			SalvarResultados(arquivo, lista, sistemaResolvido, parametros, dadosAnalise);	/*O resultado no instante t de an�lise � impresso no .TAB*/
		
		/*O instante t de an�lise � atualizado*/
		dadosAnalise.tempo_Atual += dadosAnalise.passo;										
	}
	
	/*O arquivo de sa�da .TAB � fechado*/
	arquivo.close();
	cout << "O circuito foi analisado com sucesso."<< endl;

	/*Quando o programa terminou de rodar*/
	clock_t endClock = clock();
	cout << "Tempo Gasto: " << double(endClock - startClock) / CLOCKS_PER_SEC << " s" << endl;

	/*A an�lise do circuito foi conclu�da sem problemas*/
	return(SUCESSO);
}