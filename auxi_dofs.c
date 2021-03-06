#include "phg.h"
#include "source_values.h"
#include "global_def.h"
#include <math.h>

void
get_dofs_g(DOF **dofs_Psi, DOF **dofs_g)
{
    INT i, n, idx;
    FLOAT *p_Psi[6], *p_g[6];
    FLOAT Psi11, Psi12, Psi13, Psi22, Psi23, Psi33, det;
    FLOAT g11, g12, g13, g22, g23, g33;
    GRID *g = dofs_Psi[0]->g;
    ELEMENT *e;
    
    
    INT np = dofs_Psi[0]->type->np_elem;
    ForAllElements(g, e){
        idx = e->index;
        for(i=0; i<6; i++){
            p_Psi[i] = DofElementData(dofs_Psi[i + 4], idx);
            p_g[i] = DofElementData(dofs_g[i], idx);
        }

        for(n=0; n<np; n++){
            Psi11 = *(p_Psi[0]);
            Psi12 = *(p_Psi[1]);
            Psi13 = *(p_Psi[2]);
            Psi22 = *(p_Psi[3]);
            Psi23 = *(p_Psi[4]);
            Psi33 = *(p_Psi[5]);

            det = -(Power(Psi13,2)*Psi22) + 2*Psi12*Psi13*Psi23 
                    - Psi11*Power(Psi23,2) - Power(Psi12,2)*Psi33 + Psi11*Psi22*Psi33;

            g11 = (-Power(Psi23,2) + Psi22*Psi33)/det;
            g12 = (Psi13*Psi23 - Psi12*Psi33)/det;
            g13 = (-(Psi13*Psi22) + Psi12*Psi23)/det;
            g22 = (-Power(Psi13,2) + Psi11*Psi33)/det;
            g23 = (Psi12*Psi13 - Psi11*Psi23)/det;
            g33 = (-Power(Psi12,2) + Psi11*Psi22)/det;

            *p_g[0] = g11;
            *p_g[1] = g12;
            *p_g[2] = g13;
            *p_g[3] = g22;
            *p_g[4] = g23;
            *p_g[5] = g33;

            for(i=0; i<6; i++){
                p_Psi[i]++;
                p_g[i]++;
            }
        }  
    }
}


void
get_dofs_N(DOF **dofs_Psi, DOF **dofs_g, DOF **dofs_N)
{
    INT i, n, np, idx;
    FLOAT *p_g[6], *p_Psi[4], *p_N[4];
    FLOAT Psi00, Psi01, Psi02, Psi03;
    FLOAT g11, g12, g13, g22, g23, g33;
    FLOAT N, N1, N2, N3;
    GRID *g = dofs_Psi[0]->g;
    ELEMENT *e;

    np = dofs_Psi[0]->type->np_elem;
    ForAllElements(g, e){
        idx = e->index;
        for(i=0; i<4; i++){
            p_Psi[i] = DofElementData(dofs_Psi[i], idx);
            p_N[i] = DofElementData(dofs_N[i], idx);
        }

        for(i=0; i<6; i++){
            p_g[i] = DofElementData(dofs_g[i], idx);
        }

        for(n=0;n<np;n++){
            Psi00 = *(p_Psi[0]);
            Psi01 = *(p_Psi[1]);
            Psi02 = *(p_Psi[2]);
            Psi03 = *(p_Psi[3]);

            g11 = *(p_g[0]);
            g12 = *(p_g[1]);
            g13 = *(p_g[2]);
            g22 = *(p_g[3]);
            g23 = *(p_g[4]);
            g33 = *(p_g[5]);

            //N = Power(-Psi00 + g11*Power(Psi01,2) + 2*g12*Psi01*Psi02 
            //    + g22*Power(Psi02,2) + 2*g13*Psi01*Psi03 + 2*g23*Psi02*Psi03 
            //    + g33*Power(Psi03,2),0.5);

            N1 = g11*Psi01 + g12*Psi02 + g13*Psi03;
            N2 = g12*Psi01 + g22*Psi02 + g23*Psi03;
            N3 = g13*Psi01 + g23*Psi02 + g33*Psi03;
            N = Power(-Psi00 + N1*Psi01 + N2*Psi02 + N3*Psi03,0.5);

            *p_N[0] = N;
            *p_N[1] = N1;
            *p_N[2] = N2;
            *p_N[3] = N3;

            for(i=0; i<4; i++){
                p_Psi[i]++;
                p_N[i]++;
            }
            for(i=0; i<6; i++){
                p_g[i]++;
            }
        }
    }    
}

void
get_dofs_src_and_C(DOF **dofs_var, DOF **dofs_g, DOF **dofs_N, DOF **dofs_H, DOF **dofs_deriH, DOF **dofs_src, DOF **dofs_C)
{
    INT i, n, np, idx;
    GRID *g = dofs_var[0]->g;
    ELEMENT *e;
    FLOAT *p_var[50], *p_src[50], *p_C[4], *p_g[6], *p_N[4], *p_H[4], *p_deriH[16];
    FLOAT values_var[50], values_src[50], values_g[6], values_N[4], values_H[4], values_deriH[16], values_C[4];

    np = dofs_var[0]->type->np_elem;
    ForAllElements(g, e){
        idx = e->index;
        for(i=0; i < 50; i++){
            p_var[i] = DofElementData(dofs_var[i], idx);
            p_src[i] = DofElementData(dofs_src[i], idx); 
        }
        for(i=0; i<6; i++){
            p_g[i] = DofElementData(dofs_g[i], idx);
        }
        for(i=0; i<4; i++){
            p_N[i] = DofElementData(dofs_N[i], idx);
            p_H[i] = DofElementData(dofs_H[i], idx);
        }
        for(i=0; i<16; i++){
            p_deriH[i] = DofElementData(dofs_deriH[i], idx);
        }
        for(i=0; i<4; i++){
            p_C[i] = DofElementData(dofs_C[i], idx);
        }
        //evaluate dofs values at every data point
        for(n=0;n<np;n++){
            //compute src values at data point
            for(i=0; i < 50; i++){
                values_var[i] = *(p_var[i]++);
            }
            for(i=0; i < 6; i++){
                values_g[i] = *(p_g[i]++);
            }
            for(i=0; i < 4; i++){
                values_N[i] = *(p_N[i]++);
                values_H[i] = *(p_H[i]++);
            } 
            for(i=0; i < 16; i++){
                values_deriH[i] = *(p_deriH[i]++);
            }
            get_values_src(values_var, values_g, values_N, values_H, values_deriH, values_src, values_C); 

            for(i=0; i<50; i++){
                *p_src[i] = values_src[i];
                p_src[i]++;
            }
            for(i=0; i<4; i++){
                *p_C[i] = values_C[i];
                p_C[i]++;
            }
        }
    }
}
