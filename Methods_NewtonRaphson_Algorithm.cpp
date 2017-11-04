/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém a implementação dos métodos da classe Dados_NR*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//#########################################################################################################
//#########################################################################################################

int Dados_NR::CalcularNewtonRapson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior) {
	char tipo;
	double valor_Aux;
	double novo_valor;
	double Io;

	size_t indice;
	size_t num_Variaveis = sistema.size();

	size_t contadorDiferencial = 1;
	size_t contadorInteracao = 0;

	matriz sistema_Inicial = sistema_Anterior;
	netlist net_List_Inicial = net_List;
	cout << "entrou";
	
	while (contadorDiferencial != 0 && contadorInteracao < MAX_INTERACAO_NR) {
		contadorDiferencial = 0;

		for (size_t intera = 0; intera < comp_var.size(); intera++) {
			indice = posicao_var[intera];
			// Presta atenção nessa praga
			tipo = comp_var[intera][0][0];

			switch (tipo) {
			case 'N':
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				novo_valor = CalcularValorNR(comp_var[intera], valor_Aux, Io);
				if (abs(novo_valor - net_List[indice].valor) > TOLG ) {
					EstampaNR(sistema, net_List, tipo, indice, novo_valor);
					sistema[net_List[indice].no_A][num_Variaveis] += Io - net_List[indice].Io;
					sistema[net_List[indice].no_B][num_Variaveis] -= Io - net_List[indice].Io;

					net_List[indice].Io = Io;
					contadorDiferencial++;
				}
				break;
			case '$':
				//Removendo valores anteriores
				valor_Aux = sistema_Anterior[net_List[indice].no_C][num_Variaveis] - sistema_Anterior[net_List[indice].no_D][num_Variaveis];
				novo_valor = CalcularValorNR(comp_var[intera], valor_Aux,Io);
				if (abs(novo_valor - net_List[indice].valor) > TOLG) {					
					EstampaNR(sistema, net_List, tipo, indice, novo_valor);

					contadorDiferencial++;
				}
				break;
			default:
				break;
			}
		}

		ResolverSistema(sistema, sistema_Anterior);
		contadorInteracao++;
	}
	if(contadorInteracao >1 ) cout <<"END" << contadorInteracao << endl;
	if (contadorInteracao == MAX_INTERACAO_NR) {
		for (size_t intera = 0; intera < comp_var.size(); intera++) {
			indice = posicao_var[intera];
			// Presta atenção nessa praga
			tipo = comp_var[intera][0][0];

			switch (tipo) {
			case 'N':
				EstampaNR(sistema, net_List, tipo, indice, net_List_Inicial[indice].valor);
				sistema[net_List[indice].no_A][num_Variaveis] += net_List_Inicial[indice].Io - net_List[indice].Io;
				sistema[net_List[indice].no_B][num_Variaveis] -= net_List_Inicial[indice].Io - net_List[indice].Io;

				net_List[indice].Io = net_List_Inicial[indice].Io;
				break;
			case '$':
				EstampaNR(sistema, net_List, tipo, indice, net_List_Inicial[indice].valor);
				break;
			default:
				break;
			}
		}
		sistema_Anterior = sistema_Inicial;
	}



	return(0);
}

//#########################################################################################################
//#########################################################################################################

int Dados_NR::EstampaNR(matriz &sistema, netlist &net_List, char tipo, size_t indice , double novo_valor) {
	if (tipo == 'N' || tipo == '$') {
		sistema[net_List[indice].no_A][net_List[indice].no_A] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_B] += novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_A][net_List[indice].no_B] -= novo_valor - net_List[indice].valor;
		sistema[net_List[indice].no_B][net_List[indice].no_A] -= novo_valor - net_List[indice].valor;

		net_List[indice].valor = novo_valor;
	}
	return(0);
}

//#########################################################################################################
//#########################################################################################################

double Dados_NR::CalcularValorNR(vector<string> paramNR, double valorAnterior, double &Io) {
	char tipo = paramNR[0][0];
	double auxiliar;
	
	switch (tipo){
	case 'N':
		// Primeira Reta Menor que segundo ponto
		if (valorAnterior <= stod(paramNR[5])) {
			auxiliar = (stod(paramNR[6]) - stod(paramNR[4])) / (stod(paramNR[5]) - stod(paramNR[3]));
			Io = stod(paramNR[6]) - auxiliar*(stod(paramNR[5]));
			return(auxiliar);
		}
		//Segunda Reta entre segundo e terceiro
		else if (valorAnterior <= stod(paramNR[7])) {
			auxiliar = ((stod(paramNR[8]) - stod(paramNR[6]))/(stod(paramNR[7]) - stod(paramNR[5])));
			Io = stod(paramNR[8]) - auxiliar*(stod(paramNR[7]));
			return(auxiliar);
		}
		//Terceira Reta maior que terceiro
		else {
			auxiliar = ((stod(paramNR[10]) - stod(paramNR[8]))/(stod(paramNR[9]) - stod(paramNR[7])));
			Io = stod(paramNR[10]) - auxiliar*(stod(paramNR[9]));
			return(auxiliar);
		}
		break;
	case '$':
		auxiliar = stod(paramNR[7]);

		if (valorAnterior > auxiliar) {
			return(stod(paramNR[5]));
		}

		return(stod(paramNR[6]));
		break;
	default:
		break;
	}
	return(0);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/
