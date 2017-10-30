#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//#########################################################################################################
int Dados_Analise::NumeroDeOperacoes() {
	numero_De_Analises = (size_t)floor(tempo_Final / passo);
	return(SUCESSO);
}

//#########################################################################################################
int Dados_Analise::CalcularComponentesTempo(netlist &net_List) {
	// estou dentro da fucking classe	
	string tipo;

	for (size_t index = 0; index < comp_var.size(); index++) {
		tipo = comp_var[index][0][0];
		if (tipo == "V") {
			if (comp_var[index][3] == "SIN") {
				net_List[posicao_var[index]].valor = CalcularSenoide(comp_var[index], tempo_Atual);
			}
			if (comp_var[index][3] == "PULSE") {
				net_List[posicao_var[index]].valor = CalcularPulsante(comp_var[index], tempo_Atual, passo);
			}
		}

	};


	return(SUCESSO);

}

//#########################################################################################################
int Dados_Analise::AtualizarEstampa(netlist net_List, matriz &sistema, matriz sistema_Anterior) {
	char tipo;
	size_t indice;
	double valor_Aux, quantia_aux;

	size_t num_Variaveis = sistema.size();
	for (size_t intera = 0; intera < comp_var.size(); intera++) {
		indice = posicao_var[intera];
		tipo = comp_var[intera][0][0];

		switch (tipo) {
		case 'V': // Fonte de Tensão Independente
			sistema[net_List[indice].j_x][sistema.size()] = -net_List[indice].valor;
			break;

		case 'C': // Capacitor como fonte de tensão = 0 
			//Se tiver passo variavel não fazer isso ali em baixo
			
			quantia_aux = ((2 * net_List[indice].valor) / passo);

			if (tempo_Atual == passo) {
				sistema[net_List[indice].no_A][net_List[indice].no_A] += quantia_aux - 1/RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_B][net_List[indice].no_B] += quantia_aux - 1/RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_A][net_List[indice].no_B] -= quantia_aux - 1/RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_B][net_List[indice].no_A] -= quantia_aux - 1/RESISTOR_DE_GAMBIARRA;
				
			};
			if (tempo_Atual > 0) {
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				//cout << valor_Aux << endl;
				

				if (tempo_Atual == passo) {
					valor_Aux = (valor_Aux*quantia_aux) + (valor_Aux / RESISTOR_DE_GAMBIARRA);
				}
				else {
					valor_Aux = 2*valor_Aux*quantia_aux;
				}
				//cout << valor_Aux << endl;

				sistema[net_List[indice].no_A][num_Variaveis] = -sistema[net_List[indice].no_A][num_Variaveis] + valor_Aux;
				sistema[net_List[indice].no_B][num_Variaveis] = +sistema[net_List[indice].no_A][num_Variaveis] - valor_Aux;

				//cout << sistema[net_List[indice].no_A][num_Variaveis] << endl;
				//net_List[indice].jc_0 = sistema[net_List[indice].no_A][num_Variaveis] + valor_Aux;
			}
			break;
		case 'L': // Capacitor como fonte de tensão = 0 
			//Se tiver passo variavel não fazer isso ali em baixo
			quantia_aux = (2*net_List[indice].valor / (passo));

			if (tempo_Atual == passo) {
				sistema[net_List[indice].j_x][net_List[indice].j_x] += quantia_aux - RESISTOR_DE_GAMBIARRA;
			};
			if (tempo_Atual > 0) {
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				valor_Aux = quantia_aux*sistema_Anterior[net_List[indice].j_x][num_Variaveis] + valor_Aux;

				sistema[net_List[indice].no_A][num_Variaveis] = valor_Aux;
			}
			break;
		default:
			break;
		}
	}

	return(SUCESSO);
}


