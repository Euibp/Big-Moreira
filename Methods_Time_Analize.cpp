/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém a implementação dos métodos da classe Dados_Analise*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//#########################################################################################################
//#########################################################################################################

/*Este método calcula quantas análises no tempo devem ser realizadas*/
int Dados_Analise::NumeroDeOperacoes() 
{
	numero_De_Analises = (size_t)floor(tempo_Final / passo); /*numero_De_Analises é atributo da classe e por isso não precisa ser passada como parâmetro*/
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Este método calcula o valor das fontes pulsantes e senoidais em cada instante de tempo*/
int Dados_Analise::CalcularComponentesTempo(netlist &net_List) {

	for (size_t index = 0; index < comp_var.size(); index++)
	{
		/*O tipo de componente a ser calculado é pego aqui*/
		char tipo = comp_var[index][0][0];
		/*Esse programa só admite dois componentes variáveis no tempo: Fontes de Tensão ou Corrente*/
		if (tipo == 'V' || tipo == 'I')
		{
			if (comp_var[index][3] == "SIN")
				net_List[posicao_var[index]].valor_novo = CalcularSenoide(comp_var[index], tempo_Atual);
			if (comp_var[index][3] == "PULSE")
				net_List[posicao_var[index]].valor_novo = CalcularPulsante(comp_var[index], tempo_Atual, passo);
		}
	};
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Este método atualiza a estampa dos componentes variáveis no tempo durante a análise*/
int Dados_Analise::AtualizarEstampa(netlist &net_List, matriz &sistema, matriz sistema_Anterior) {
	char tipo;
	size_t indice;
	double valor_Aux, quantia_aux;
	size_t num_Variaveis = sistema.size();

	for (size_t intera = 0; intera < comp_var.size(); intera++) {
		indice = posicao_var[intera];
		// Presta atenção nessa praga
		tipo = comp_var[intera][0][0];

		switch (tipo) {
		case 'V': // Fonte de Tensão Independente
			sistema[net_List[indice].j_x][sistema.size()] -= (net_List[indice].valor_novo - net_List[indice].valor) ;
			net_List[indice].valor = net_List[indice].valor_novo;
			break;

		case 'I': // Fonte de Tensão Independente
			sistema[net_List[indice].no_A][num_Variaveis] -= (net_List[indice].valor_novo - net_List[indice].valor);
			sistema[net_List[indice].no_B][num_Variaveis] += (net_List[indice].valor_novo - net_List[indice].valor);
			net_List[indice].valor = net_List[indice].valor_novo;
			break;

		case 'C': 
			quantia_aux = ((2 * net_List[indice].valor) / passo);

			if (tempo_Atual == passo) {
				sistema[net_List[indice].no_A][net_List[indice].no_A] += quantia_aux - 1 / RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_B][net_List[indice].no_B] += quantia_aux - 1 / RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_A][net_List[indice].no_B] -= quantia_aux - 1 / RESISTOR_DE_GAMBIARRA;
				sistema[net_List[indice].no_B][net_List[indice].no_A] -= quantia_aux - 1 / RESISTOR_DE_GAMBIARRA;
			};

			if (tempo_Atual > 0) {
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				//cout << valor_Aux;
				//cout << " :: " << sistema[net_List[indice].no_A][num_Variaveis] << endl;
				//cout << " :: " << net_List[indice].jc_0;

				if (tempo_Atual == passo) {
					valor_Aux = (valor_Aux*quantia_aux) + (valor_Aux / RESISTOR_DE_GAMBIARRA);
				}
				else {
					sistema[net_List[indice].no_A][num_Variaveis] -= net_List[indice].jc_0;
					sistema[net_List[indice].no_B][num_Variaveis] += net_List[indice].jc_0;


					// calcula J atual
					net_List[indice].jc_0 = valor_Aux*quantia_aux - net_List[indice].jc_0;

					// Calcula tensão atual
					valor_Aux = valor_Aux*quantia_aux + net_List[indice].jc_0;

				}
				sistema[net_List[indice].no_A][num_Variaveis] += valor_Aux;
				sistema[net_List[indice].no_B][num_Variaveis] -= valor_Aux;

				net_List[indice].jc_0 = valor_Aux;

				//cout << sistema[net_List[indice].no_A][num_Variaveis] << endl;

			}
			break;
		case 'L': // Capacitor como fonte de tensão = 0 
			//Se tiver passo variavel não fazer isso ali em baixo
			quantia_aux = (2 * net_List[indice].valor / (passo));

			if (tempo_Atual > 0) {

				if (tempo_Atual == passo) {
					sistema[net_List[indice].j_x][net_List[indice].j_x] += quantia_aux - (1 / RESISTOR_DE_GAMBIARRA);

				};
				sistema[net_List[indice].j_x][num_Variaveis] -= net_List[indice].jc_0;
				valor_Aux = sistema_Anterior[net_List[indice].no_A][num_Variaveis] - sistema_Anterior[net_List[indice].no_B][num_Variaveis];
				valor_Aux = quantia_aux*sistema_Anterior[net_List[indice].j_x][num_Variaveis] + valor_Aux;


				sistema[net_List[indice].j_x][num_Variaveis] += valor_Aux;
				net_List[indice].jc_0 = valor_Aux;
			}
			break;
		default:
			break;

		}
	}
	return(SUCESSO);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/