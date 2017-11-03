#include "stdafx.h"
#include "Header_CircuitSimulator.h"

int Dados_NR::CalcularNewtonRapson(netlist &net_List, matriz &sistema, matriz &sistema_Anterior) {
	char tipo;
	double valor_Aux;
	double novo_valor;
	size_t indice;
	size_t num_Variaveis = sistema.size();

	size_t contadorDiferencial = 1;
	size_t contadorInteracao = 0;

	while (contadorDiferencial != 0 || contadorInteracao <50) {
		contadorDiferencial = 0;

		for (size_t intera = 0; intera < comp_var.size(); intera++) {
			indice = posicao_var[intera];
			// Presta atenção nessa praga
			tipo = comp_var[intera][0][0];

			switch (tipo) {
			case '$':
				//Removendo valores anteriores
				valor_Aux = sistema_Anterior[net_List[indice].no_C][num_Variaveis] - sistema_Anterior[net_List[indice].no_D][num_Variaveis];
				novo_valor = CalcularValorNR(comp_var[intera], valor_Aux);
				if (novo_valor != net_List[indice].valor) {
					sistema[net_List[indice].no_A][net_List[indice].no_A] += novo_valor - net_List[indice].valor;
					sistema[net_List[indice].no_B][net_List[indice].no_B] += novo_valor - net_List[indice].valor;
					sistema[net_List[indice].no_A][net_List[indice].no_B] -= novo_valor - net_List[indice].valor;
					sistema[net_List[indice].no_B][net_List[indice].no_A] -= novo_valor - net_List[indice].valor;

					net_List[indice].valor = novo_valor;
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

	return(0);
}


double Dados_NR::CalcularValorNR(vector<string> paramNR, double valorAnterior) {
	char tipo = paramNR[0][0];

	double auxiliar;
	switch (tipo){
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

}