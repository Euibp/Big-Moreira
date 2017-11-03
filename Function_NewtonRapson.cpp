#include "stdafx.h"
#include "Header_CircuitSimulator.h"

double CalcularNewtonRapson(vector<string> paramCompNR, double valorAnterior) {
	char tipo;
	tipo = paramCompNR[0][0];

	double auxiliar;

	switch (tipo) {
	case '$':
		auxiliar = stod(paramCompNR[7]);

		if (valorAnterior > auxiliar) {
			return(stod(paramCompNR[5]));
		}
		
		return(stod(paramCompNR[6]));
		break;
	}

	return(0);
}