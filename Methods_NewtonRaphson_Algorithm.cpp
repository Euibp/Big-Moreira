/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém a implementação dos métodos da classe Dados_NR*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

/*Macros Utilizadas para análises de circuitos não lineares*/
#define MAX_INTERACAO_NR 20				/*Número máximo de interações utilizando Newton-Raphson*/
#define MAX_INTERACAO_GMIN_STEPPING 20	/*Número máximo de interações utilizando Gmin Stepping*/
#define GMIN_MAXIMO 100					/*Máxima condutância posta em paralelo com um componente não linear em Gmin Stepping*/
#define GMIN_MINIMO 1e-12				/*Mínima condutância posta em paralelo com um componente não linear em Gmin Stepping*/
#define FATOR_INICIAL 10				/*Maior fator de divisão de Gmin*/
#define FATOR_DIVISAO_MINIMO 1.01		/*Menor fator de divisão de Gmin*/

/*Esta struct contém parâmetros auxiliares utilizados em Gmin Stepping*/
struct gminParam {
	bool Gmin;							/*Variável auxiliar de condutância posta em paralelo com ramos não lineares*/
	bool convergencia;					/*Flag que marca se uma análise convergiu ou não convergiu*/
	double fator;						/*Variável auxiliar do fator de divisão de Gmin*/
};

//#########################################################################################################
//#########################################################################################################

/*Este método calcula a próxima aproximação quando no Método de Newton-Raphson*/
int Dados_NR::CalcularNewtonRapson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior) {
	/*Variáveis utilizadas*/
	int erro;																	/**/
	size_t indice;																/*Variável auxiliar utilizada em loops*/
	char tipo;																	/**/

	size_t contadorInteracao = 0;												/**/
	size_t contadorDiferencial = 1;												/**/
	bool Gmin_Comecou = false;													/*Flag que indica se Gmin Stepping começou a ser utilizado*/ 

	matriz sistema_Inicial = sistema_Anterior;									/**/
	netlist net_List_Inicial = net_List;										/**/

	vector<bool> verifica_Convergencia(comp_var.size(), true);					/**/
	vector<double> fator_divisao(comp_var.size(), FATOR_INICIAL);				/**/

	/*Início do cálculo da próxima aproximação*/
	while (contadorDiferencial != 0) {
		contadorInteracao = InteracaoNR(sistema, net_List, sistema_Anterior, verifica_Convergencia);
//		if (contadorInteracao > 1) std::cout << "END :: " << contadorInteracao << endl;

		if (contadorInteracao <= MAX_INTERACAO_NR && Gmin_Comecou == false)
			break;
		contadorDiferencial = 0;
		Gmin_Comecou = true;

		for (size_t indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
				indice = posicao_var[indice_NL];
				// Presta atenção nessa praga
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
		// Presta atenção nessa praga
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

/*Este método atualiza a estampa dos componentes não lineares durante a análise*/
int Dados_NR::EstampaNR(matriz &sistema, netlist &net_List, char tipo, size_t indice, double novo_valor) {
	
	/*Esse loop verifica se um componente chave ($) ou resistor linear por partes está presente no circuito*/
	/*Em caso positivo, a estampa do componente é atualizada. Para ambos os componentes a estampa atualizada*/
	/*é uma condutância.*/
	if (tipo == 'N' || tipo == '$')
	{
		sistema[net_List[indice].no_A][net_List[indice].no_A] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_B] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_A][net_List[indice].no_B] -= novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_A] -= novo_valor - net_List[indice].valor;
		net_List[indice].valor = novo_valor;				/*O valor da condutância do componente não linear é atualizado*/
		return (SUCESSO)
	}
	/*Caso não existam chaves ou resistores não lineares, o método retorna esse código*/
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

/*Esse método calcula o próximo valor de um componente não linear*/
double Dados_NR::CalcularValorNR(vector<string> paramNR, double valorAnterior, double &Io) {
	
	/*Variáveis utilizadas*/
	double condutanciaDoComponente = 0;	/*Condutância do componente retornada pelo método*/

	/*Para cada tipo de componente não linear, o próximo valor é calculado de uma forma diferente*/
	char tipo = paramNR[0][0];														
	switch (tipo) {
	
	/*Se o componente não linear for um resistor linear por partes*/
	/*Um resistor linear por partes é definido por três retas, o que significa dizer que há três possíveis valores para sua resistência*/
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
	
	/*Se o componente não linear for uma chave*/
	case '$':
		condutanciaDoComponente = stod(paramNR[7]);

		if (valorAnterior > condutanciaDoComponente) {
			return(stod(paramNR[5]));
		}

		return(stod(paramNR[6]));
		break;

	/*Se o componente não linear não for nem um resistor linear por partes nem uma chave*/
	default:
		break;
	}

	return(ERRO_COMPONENTE_NAO_CALCULADO);
}

//#########################################################################################################
//#########################################################################################################

int Dados_NR::InteracaoNR(matriz &sistema, netlist &net_List, matriz &sistema_Anterior, vector<bool> &verifica_Convergencia) {
	/*Varáveis utilizadas*/
	char tipo;										/*Armazena o tipo de componente não linear que está sendo tratado*/
	double valor_Aux;
	double novo_valor;
	double Io;

	size_t indice;
	site_t indice_NL;
	size_t num_Variaveis = sistema.size();
	size_t contadorDiferencial = 1;
	size_t contadorInteracao = 0;					/*Variável auxiliar que armazena o número de iterações no método de Newton-Raphson*/

	/*Esse loop é executado enquanto o número máximo de iterações não é atingido*/
	for (contadorInteracao = 0; contadorInteracao <= MAX_INTERACAO_NR; contadorInteracao++) {
		contadorDiferencial = 0;

		/*Esse loop é executado tantas vezes quantos forem os elementos não lineares do circuito*/
		for (indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
			indice = posicao_var[indice_NL];
			// Presta atenção nessa praga
			tipo = comp_var[indice_NL][0][0];															/*O tipo do componente não linear a ser tratado é obtido aqui*/
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
			
			/*Caso não for uma chave e nem um resistor linear por partes*/
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



//// Parte que resolve Problemas de Estabilização
//for (size_t indice_NL = 0; indice_NL < comp_var.size(); indice_NL++) {
//	indice = posicao_var[indice_NL];
//	// Presta atenção nessa praga
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