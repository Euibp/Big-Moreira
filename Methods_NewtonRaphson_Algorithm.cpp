/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m a implementa��o dos m�todos da classe Dados_NR*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

/*Macros Utilizadas para an�lises de circuitos n�o lineares*/
#define MAX_INTERACAO_NR 20				/*N�mero m�ximo de intera��es utilizando Newton-Raphson*/
#define MAX_INTERACAO_GMIN_STEPPING 20	/*N�mero m�ximo de intera��es utilizando Gmin Stepping*/
#define GMIN_MAXIMO 100					/*M�xima condut�ncia posta em paralelo com um componente n�o linear em Gmin Stepping*/
#define GMIN_MINIMO 1e-12				/*M�nima condut�ncia posta em paralelo com um componente n�o linear em Gmin Stepping*/
#define FATOR_INICIAL 10				/*Maior fator de divis�o de Gmin*/
#define FATOR_DIVISAO_MINIMO 1.01		/*Menor fator de divis�o de Gmin*/

/*Esta struct cont�m par�metros auxiliares utilizados em Gmin Stepping*/
struct gminParam {
	bool Gmin;							/*Vari�vel auxiliar de condut�ncia posta em paralelo com ramos n�o lineares*/
	bool convergencia;					/*Flag que marca se uma an�lise convergiu ou n�o convergiu*/
	double fator;						/*Vari�vel auxiliar do fator de divis�o de Gmin*/
};

//#########################################################################################################
//#########################################################################################################

/*Este m�todo calcula a pr�xima aproxima��o quando no M�todo de Newton-Raphson*/
int Dados_NR::CalcularNewtonRapson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior) {
	/*Vari�veis utilizadas*/
	int erro;																	/**/
	size_t indice;																/*Vari�vel auxiliar utilizada em loops*/
	char tipo;																	/**/

	size_t contadorInteracao = 0;												/**/
	size_t contadorDiferencial = 1;												/**/
	bool Gmin_Comecou = false;													/*Flag que indica se Gmin Stepping come�ou a ser utilizado*/ 

	matriz sistema_Inicial = sistema_Anterior;									/**/
	netlist net_List_Inicial = net_List;										/**/

	vector<bool> verifica_Convergencia(comp_var.size(), true);					/**/
	vector<double> fator_divisao(comp_var.size(), FATOR_INICIAL);				/**/

	/*In�cio do c�lculo da pr�xima aproxima��o*/
	while (contadorDiferencial != 0) {
		contadorInteracao = InteracaoNR(sistema, net_List, sistema_Anterior, verifica_Convergencia);
//		if (contadorInteracao > 1) std::cout << "END :: " << contadorInteracao << endl;

		if (contadorInteracao <= MAX_INTERACAO_NR && Gmin_Comecou == false)
			break;
		contadorDiferencial = 0;
		Gmin_Comecou = true;

		for (size_t indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
				indice = posicao_var[indice_NL];
				// Presta aten��o nessa praga
				tipo = comp_var[indice_NL][0][0];
			switch (tipo) {
			case 'N':
				erro = GminStep(sistema, net_List, tipo, indice, verifica_Convergencia[indice_NL],fator_divisao[indice_NL]);
				cout << "erro" << erro << endl;
				if (erro != ESTABILIZOU) contadorDiferencial++;
				if (erro == ERRO_DE_ESTABILIZACAO) return(erro);
				verifica_Convergencia[indice_NL] = true;
				break;
			default:
				break;
			}
		}
		ResolverSistema(sistema, sistema_Anterior);
		cout << "GminContador ::" << contadorDiferencial<<endl;
	}
	for (size_t indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
		indice = posicao_var[indice_NL];
		// Presta aten��o nessa praga
		tipo = comp_var[indice_NL][0][0];
		double valor_Aux;
		double novo_valor;
		double Io;
		size_t num_Variaveis = sistema.size();

		switch (tipo) {
		case 'N':
			valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
			novo_valor = CalcularValorNR(comp_var[indice_NL], valor_Aux, Io);
			cout << novo_valor << endl;
			break;
		default:
			break;
		}
	}

	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Este m�todo atualiza a estampa dos componentes n�o lineares durante a an�lise*/
int Dados_NR::EstampaNR(matriz &sistema, netlist &net_List, char tipo, size_t indice, double novo_valor) {
	
	/*Esse loop verifica se um componente chave ($) ou resistor linear por partes est� presente no circuito*/
	/*Em caso positivo, a estampa do componente � atualizada. Para ambos os componentes a estampa atualizada*/
	/*� uma condut�ncia.*/
	if (tipo == 'N' || tipo == '$')
	{
		sistema[net_List[indice].no_A][net_List[indice].no_A] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_B] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_A][net_List[indice].no_B] -= novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_A] -= novo_valor - net_List[indice].valor;
		net_List[indice].valor = novo_valor;				/*O valor da condut�ncia do componente n�o linear � atualizado*/
		return (SUCESSO)
	}
	/*Caso n�o existam chaves ou resistores n�o lineares, o m�todo retorna esse c�digo*/
	return(ERRO_ESTAMPA_NAO_LINEAR);
}

//#########################################################################################################
//#########################################################################################################

int Dados_NR::GminStep(matriz &sistema, netlist &net_List, char tipo, size_t indice, bool convergencia, double &fator) {
	double novo_Gmin;

	if (net_List[indice].gmin < GMIN_MINIMO && convergencia == false)  novo_Gmin = GMIN_MAXIMO;
	else {
		if (convergencia == true) { // divid por 10
			fator = 10;
			cout << " FATOR ::" << fator << endl;
		}
		if (convergencia == false) {
			cout << " FATOR ::" << fator << endl;
			net_List[indice].gmin = net_List[indice].gmin * fator;
			fator = sqrt(fator);
			cout <<" FATOR ::" << fator<< endl;
			if (fator < FATOR_DIVISAO_MINIMO) return(ERRO_DE_ESTABILIZACAO);
		}
		novo_Gmin = net_List[indice].gmin / fator;

		if (novo_Gmin < GMIN_MINIMO) {
			if (convergencia == false) return(ERRO_DE_ESTABILIZACAO);
			net_List[indice].gmin = novo_Gmin;
			return(ESTABILIZOU);
		}
	}
	

	sistema[net_List[indice].no_A][net_List[indice].no_A] += novo_Gmin - net_List[indice].gmin;
	sistema[net_List[indice].no_B][net_List[indice].no_B] += novo_Gmin - net_List[indice].gmin;
	sistema[net_List[indice].no_A][net_List[indice].no_B] -= novo_Gmin - net_List[indice].gmin;
	sistema[net_List[indice].no_B][net_List[indice].no_A] -= novo_Gmin - net_List[indice].gmin;

	net_List[indice].gmin = novo_Gmin;

	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Esse m�todo calcula o pr�ximo valor de um componente n�o linear*/
double Dados_NR::CalcularValorNR(vector<string> paramNR, double valorAnterior, double &Io) {
	
	/*Vari�veis utilizadas*/
	double condutanciaDoComponente = 0;	/*Condut�ncia do componente retornada pelo m�todo*/

	/*Para cada tipo de componente n�o linear, o pr�ximo valor � calculado de uma forma diferente*/
	char tipo = paramNR[0][0];														
	switch (tipo) {
	
	/*Se o componente n�o linear for um resistor linear por partes*/
	/*Um resistor linear por partes � definido por tr�s retas, o que significa dizer que h� tr�s poss�veis valores para sua resist�ncia*/
	case 'N':
		
		/*Se o componente for definido pela primeira reta, definida pelo primeiro e pelo segundo ponto*/		
		if (valorAnterior <= stod(paramNR[5])) {
			condutanciaDoComponente = (stod(paramNR[6]) - stod(paramNR[4])) / (stod(paramNR[5]) - stod(paramNR[3]));
			Io = stod(paramNR[6]) - condutanciaDoComponente*(stod(paramNR[5]));
			return(condutanciaDoComponente);
		}

		/*Se o componente for definido pela segunda reta, definida pelo segundo e pelo terceiro ponto*/
		else if (valorAnterior <= stod(paramNR[7])) {
			condutanciaDoComponente = ((stod(paramNR[8]) - stod(paramNR[6])) / (stod(paramNR[7]) - stod(paramNR[5])));
			Io = stod(paramNR[8]) - condutanciaDoComponente*(stod(paramNR[7]));
			return(condutanciaDoComponente);
		}

		/*Se o componente for definido pela terceira reta, definida pelo segundo e pelo terceiro ponto*/
		else {
			condutanciaDoComponente = ((stod(paramNR[10]) - stod(paramNR[8])) / (stod(paramNR[9]) - stod(paramNR[7])));
			Io = stod(paramNR[10]) - condutanciaDoComponente*(stod(paramNR[9]));
			return(condutanciaDoComponente);
		}
		break;
	
	/*Se o componente n�o linear for uma chave*/
	case '$':
		condutanciaDoComponente = stod(paramNR[7]);

		if (valorAnterior > condutanciaDoComponente) {
			return(stod(paramNR[5]));
		}

		return(stod(paramNR[6]));
		break;

	/*Se o componente n�o linear n�o for nem um resistor linear por partes nem uma chave*/
	default:
		break;
	}

	return(ERRO_COMPONENTE_NAO_CALCULADO);
}

//#########################################################################################################
//#########################################################################################################

int Dados_NR::InteracaoNR(matriz &sistema, netlist &net_List, matriz &sistema_Anterior, vector<bool> &verifica_Convergencia) {
	/*Var�veis utilizadas*/
	char tipo;										/*Armazena o tipo de componente n�o linear que est� sendo tratado*/
	double valor_Aux;
	double novo_valor;
	double Io;

	size_t indice;
	site_t indice_NL;
	size_t num_Variaveis = sistema.size();
	size_t contadorDiferencial = 1;
	size_t contadorInteracao = 0;					/*Vari�vel auxiliar que armazena o n�mero de itera��es no m�todo de Newton-Raphson*/

	/*Esse loop � executado enquanto o n�mero m�ximo de itera��es n�o � atingido*/
	for (contadorInteracao = 0; contadorInteracao <= MAX_INTERACAO_NR; contadorInteracao++) {
		contadorDiferencial = 0;

		/*Esse loop � executado tantas vezes quantos forem os elementos n�o lineares do circuito*/
		for (indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
			indice = posicao_var[indice_NL];
			// Presta aten��o nessa praga
			tipo = comp_var[indice_NL][0][0];															/*O tipo do componente n�o linear a ser tratado � obtido aqui*/
			switch (tipo) {
			
			/*Caso for um resistor linear por partes*/
			case 'N':
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				novo_valor = CalcularValorNR(comp_var[indice_NL], valor_Aux, Io);
//				cout << novo_valor << " // " << net_List[indice].valor << endl;

				if (abs(novo_valor - net_List[indice].valor) > TOLG) {
					EstampaNR(sistema, net_List, tipo, indice, novo_valor);
					sistema[net_List[indice].no_A][num_Variaveis] += Io - net_List[indice].Io;
					sistema[net_List[indice].no_B][num_Variaveis] -= Io - net_List[indice].Io;

					net_List[indice].Io = Io;
					contadorDiferencial++;
					if (contadorInteracao == MAX_INTERACAO_NR) {
						verifica_Convergencia[indice_NL] = false;
						//indice_comp_instaveis.push_back(indice);
					}
				}
				else if (contadorInteracao == MAX_INTERACAO_NR && (net_List[indice].no_A == 0 || net_List[indice].no_B == 0)) {
					//indice_comp_instaveis.push_back(indice);
				}
				break;
			
			/*Caso for uma chave*/
			case '$':
				//Removendo valores anteriores
				valor_Aux = sistema_Anterior[net_List[indice].no_C][num_Variaveis] - sistema_Anterior[net_List[indice].no_D][num_Variaveis];
				novo_valor = CalcularValorNR(comp_var[indice_NL], valor_Aux, Io);
				if (abs(novo_valor - net_List[indice].valor) > TOLG) {
					EstampaNR(sistema, net_List, tipo, indice, novo_valor);

					contadorDiferencial++;
				}
				break;
			
			/*Caso n�o for uma chave e nem um resistor linear por partes*/
			default:
				break;
			}
		}

		ResolverSistema(sistema, sistema_Anterior);
		//contadorInteracao++;
		if (contadorDiferencial == 0) break;
	}
	return(contadorInteracao);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/



//// Parte que resolve Problemas de Estabiliza��o
//for (size_t indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
//	indice = posicao_var[indice_NL];
//	// Presta aten��o nessa praga
//	tipo = comp_var[indice_NL][0][0];
//	switch (tipo) {
//	case 'N':
//		EstampaNR(sistema, net_List, tipo, indice, net_List_Inicial[indice].valor);
//		sistema[net_List[indice].no_A][num_Variaveis] += net_List_Inicial[indice].Io - net_List[indice].Io;
//		sistema[net_List[indice].no_B][num_Variaveis] -= net_List_Inicial[indice].Io - net_List[indice].Io;

//		net_List[indice].Io = net_List_Inicial[indice].Io;
//		cout << net_List[indice].Io << " :: " << net_List[indice].valor << endl;
//		break;
//	case '$':
//		EstampaNR(sistema, net_List, tipo, indice, net_List_Inicial[indice].valor);
//		break;
//	default:
//		break;
//	}
//}

//