/*test the grad module*/

#include <math.h>
#include "phg.h"

#define Power Pow
#define M 0.5
#define R Pow(x*x + y*y + z*z, 0.5)
static void
func_Psi33(FLOAT x, FLOAT y, FLOAT z, FLOAT *value)
{
    *value = Pow(1.0+M/R, 2.0) + (1.0+M/R)/(1-M/R)*Pow(M, 2.0)/Pow(R, 4.0)*Pow(z, 2.0);
}

static void
func_Phi333(FLOAT x, FLOAT y, FLOAT z, FLOAT *value)
{
    *value = (-2.*Power(M,3.)*Power(z,3.)*Power(Power(x,2) + Power(y,2) + Power(z,2),7.5) - 2.*Power(M,3)*z*Power(Power(x,2) + Power(y,2) + 
    Power(z,2),8.5)*Power(1. + M/Power(Power(x,2) + Power(y,2) + Power(z,2),0.5),1.) + 
     4.*Power(M,2)*z*Power(Power(x,2) + Power(y,2) + Power(z,2),9.)*Power(1. + M/Power(Power(x,2) + Power(y,2) + Power(z,2),0.5),1.) - 
     2.*M*z*Power(Power(x,2) + Power(y,2) + Power(z,2),9.5)*Power(1. + M/Power(Power(x,2) + Power(y,2) + Power(z,2),0.5),1.) + 
     Power(M,4.)*(4.*Power(z,3.)*Power(Power(x,2) + Power(y,2) + Power(z,2),7.) - 2.*Power(z,1.)*Power(Power(x,2) + Power(y,2) + Power(z,2),8.)) + 
     Power(M,2.)*(-4.*Power(z,3.)*Power(Power(x,2) + Power(y,2) + Power(z,2),8.) + 2.*Power(z,1.)*Power(Power(x,2) + Power(y,2) + Power(z,2),9.)))/
   (Power(Power(x,2) + Power(y,2) + Power(z,2),10.)*Power(-1.*M + Power(Power(x,2) + Power(y,2) + Power(z,2),0.5),2));
}

dgQuadFaceNeighDofDotBas(ELEMENT *e, DOF *u, int face, int N, ELEMENT *neigh_e, 
DOF *v, int neigh_face, int order)
{
    int i, j, e_v[NVert], neigh_v[NVert];
    FLOAT d, lambda[Dim + 1], neigh_lambda[Dim + 1];
    FLOAT *dof, *buffer;
    const FLOAT *bas, *p, *w;
    QUAD *quad;
    DOF_TYPE *type;

    assert(!SpecialDofType(u->type));
    assert(face >= 0 && face <= 3);

    type = (DofIsHP(u) ? u->hp->info->types[u->hp->max_order] : u->type);

    if (order < 0) {
	    i = DofTypeOrder(v, neigh_e);
	    j = DofTypeOrder(u, e);
	    if (i < 0)
	        i = j;
	    order = i + j;
    }
    quad = phgQuadGetQuad2D(order);

    /*有更好的实现方法吗？*/
    /*e_v[i]: face号面上的i号顶点在本单元的编号；
      neigh_v[i]: 该顶点在face面的相邻单元的编号*/
    for (i=0; i < NVert - 1; i++) {
        e_v[i] = GetFaceVertex(face, i);
        for (j=0; j < NVert; j++) {
            if (neigh_e->verts[j] == e->verts[e_v[i]]) {
                neigh_v[i] = j;
                break; 
            } 
        }
    }
    lambda[face] = 0.;
    neigh_lambda[neigh_face] = 0.;

    buffer = phgAlloc(sizeof(*buffer));
    p = quad->points;
    w = quad->weights;

    d = 0.;
    for (i = 0; i < quad->npoints; i++) {
        for (j = 0; j < NVert - 1; j++) {
        lambda[e_v[j]] = *(p++);
        neigh_lambda[neigh_v[j]] = lambda[e_v[j]];
        }
        
        dof = phgDofEval(v, neigh_e, neigh_lambda, buffer);
        bas = (FLOAT *)type->BasFuncs(u, e, N, N + 1, lambda);
        d += *(bas++) * *(dof++) * *(w++);
    } 
    phgFree(buffer);

    return d * phgGeomGetFaceArea(u->g, e, face);
}


static void
split_dof(DOF *dof_A, DOF *dof_B, DOF *dof_C)
{
    GRID *g = dof_A->g;
    ELEMENT *e;
    FLOAT *p_A, *p_B, *p_C;
    INT np = dof_A->type->np_elem;
    INT idx, n;
    ForAllElements(g, e){
        idx = e->index;
        p_A = DofElementData(dof_A, idx);
        p_B = DofElementData(dof_B, idx);
        p_C = DofElementData(dof_C, idx);
        for(n=0;n<np;n++){
           *p_B++ = *p_A++;
           *p_C++ = *p_A++;
        }
    } 
}

void
phgQuadDofGradBas(ELEMENT *e, DOF *p, DOF *v, int m, int order, FLOAT *values)
{
    int i, j;
    const FLOAT *g1, *g2, *w;
    FLOAT d1, d2, d3, vol;
    QUAD *quad;

    assert(!SpecialDofType(v->type));
    assert(p->dim == 1);

    if (order < 0) {                                                                                          
         i = DofTypeOrder(p, e);                                                                         
         j = DofTypeOrder(v, e);                                                                               
         order = i + j;
     } 
	 
    quad = phgQuadGetQuad3D(order);
    g1 = phgQuadGetDofValues(e, p, quad);
 
    g2 = phgQuadGetBasisGradient(e, v, m, quad);
    w = quad->weights;
    
    d1 = d2 = d3 = 0.;
    for (i = 0; i < quad->npoints; i++) {
        d1 += *(g1) * (*g2++) * (*w);
        d2 += *(g1) * (*g2++) * (*w);
        d3 += *(g1) * (*g2++) * (*w);
        g1++;
        w++;
    }

    vol = phgGeomGetVolume(p->g, e);
    values[0] = d1 * vol;
    values[1] = d2 * vol;
    values[2] = d3 * vol;
}

static void
set_neighbour_data(DOF *u, ELEMENT *e, INT s, DOF *neigh_u, NEIGHBOUR_DATA *nd)
{ 
    assert(e->bound_type[s] & INTERIOR); 
    SHORT i, nbasface;
    FLOAT *ele_data;

    ele_data = DofElementData(neigh_u, e->index);
    nbasface = phgDofNeighbourNBas(nd, e, s, NULL);
    SHORT bases[nbasface];
    phgDofGetBasesOnFace(u, e, s, bases);
    for(i=0;i<nbasface;i++){
        ele_data[bases[i]] = *phgDofNeighbourData(nd, e, s, i, NULL);
    }
}

static void
reset_neighbour_data(DOF *u, ELEMENT *e, INT s, DOF *neigh_u, NEIGHBOUR_DATA *nd)
{
    assert(e->bound_type[s] & INTERIOR); 
    SHORT i, nbasface;
    FLOAT *ele_data;

    ele_data = DofElementData(neigh_u, e->index);
    nbasface = phgDofNeighbourNBas(nd, e, s, NULL);
    SHORT bases[nbasface];
    phgDofGetBasesOnFace(u, e, s, bases);
    for(i=0;i<nbasface;i++){
        ele_data[bases[i]] = 0.;   
    }
}


static void
build_linear_system(SOLVER *solver, DOF *u_h, DOF *dof_bdry, int coord)
{
    int n, k, i, j, s, in_out;
    GRID *g = u_h->g;
    DOF *neigh_u;
    ELEMENT *e;
    FLOAT val_ext, val_int, mass_term, boundary_term, stiff_term[3];

    NEIGHBOUR_DATA *nd = phgDofInitNeighbourData(u_h, NULL); 
   
    neigh_u = phgDofNew(g, u_h->type, 1, "neigh_u", DofInterpolation); 

    ForAllElements(g, e){ 
        int nbas = DofGetNBas(u_h, e); 
        int I[2*nbas];
        
        for(n = 0; n<nbas; n++){
            
            /*stiffness part*/ 
            phgQuadDofGradBas(e, u_h, u_h, n, QUAD_DEFAULT, stiff_term);

            /*mass matrix*/
            for(in_out = 0; in_out < 2; in_out++){
                /*I[]建立了一个映射，每个单元的自由度在线性解法器中被给予一个编号*/
                i = 2*n + in_out;//i是e单元中基函数编号为n、flux编号为in_out对应的自由度编号 

                I[i] = phgSolverMapE2L(solver, 0, e, i);

                /*mass part*/
                for(k = 0; k <= n; k++){
                    mass_term = phgQuadBasDotBas(e, u_h, n, u_h, k, QUAD_DEFAULT);
                    j = 2*k + in_out;
                    /*添加到解法器中矩阵的相应项*/
                    phgSolverAddMatrixEntry(solver, I[i], I[j], mass_term);
                    if(k < n){
                        phgSolverAddMatrixEntry(solver, I[j], I[i], mass_term);
                    }
                }

                /*添加stiffness到解法器相应右端项*/
                phgSolverAddRHSEntry(solver, I[i], - stiff_term[coord]);
            
                /*boundary term, using numerical flux*/ 
                for (s=0; s<4; s++){
                    const FLOAT *normal = phgGeomGetFaceOutNormal(g, e, s);                        

                    val_int = phgQuadFaceDofDotBas(e, s, u_h, DOF_PROJ_NONE, 
                                u_h, n, QUAD_DEFAULT); //计算u^-
                   
                    /*分是否边界面来计算u^+*/ 
                    if(e->bound_type[s] & INTERIOR){//如果s是内部面,则利用neigh_u来计算u^+
                        //set_neighbour_data(u_h, e, s, neigh_u, nd); 
                        //val_ext = phgQuadFaceDofDotBas(e, s, neigh_u, DOF_PROJ_NONE, 
                        //        u_h, n, QUAD_DEFAULT); 
                        //reset_neighbour_data(u_h, e, s, neigh_u, nd); 
                        ELEMENT *neigh_e = e->neighbours[s];
                        int ss;
                        for(ss=0; ss<4; ss++){
                            if(e->faces[s]==neigh_e->faces[ss]){
                                break;
                            }
                        }
                        val_ext = dgQuadFaceNeighDofDotBas(e, u_h, s, n, neigh_e, u_h, ss, QUAD_DEFAULT);  
                       
                    } 
                    else{//s是边界面时,则施加边界条件 
                        val_ext = phgQuadFaceDofDotBas(e, s, dof_bdry, DOF_PROJ_NONE, 
                                u_h, n, QUAD_DEFAULT);
                        
                    }
                    val_ext = phgQuadFaceDofDotBas(e, s, u_h, DOF_PROJ_NONE, 
                                u_h, n, QUAD_DEFAULT); 


                    boundary_term = 0.5 * normal[coord] * (val_ext + val_int) + (0.5 - in_out) * 
                            Fabs(normal[coord]) * (val_ext - val_int);//数值通量, in_out = 0 对应于取plus  
                    /*添加到解法器相应右端项*/
                    phgSolverAddRHSEntry(solver, I[i], boundary_term);
                } 
            }
        }
    }
    phgDofReleaseNeighbourData(&nd); 
    phgDofFree(&neigh_u);
}

static void
dgHJGradDof(DOF *u_h, DOF *dof_bdry, DOF *dof_grad, int coord)
{
    SOLVER *solver;
    solver = phgSolverCreate(SOLVER_DEFAULT, dof_grad, NULL);
    
    build_linear_system(solver, u_h, dof_bdry, coord);

    MAT *mat_M = phgSolverGetMat(solver);                                                                        
    phgMatDumpMATLAB(mat_M, "mat", "mat.m");    
    phgVecDumpMATLAB(solver->rhs, "rhs", "rhs.m");
    phgSolverSolve(solver, TRUE, dof_grad, NULL);
    phgSolverDestroy(&solver);
}

static void
get_dof_grad_hat(DOF *dof_grad)
{
    
    FLOAT grad_plus, grad_minus;
    FLOAT *p;
    GRID *g = dof_grad->g;
    ELEMENT *e;
    int i, idx;
    
    int np = dof_grad->type->np_elem;
    ForAllElements(g, e){
        idx = e->index;
        p = DofElementData(dof_grad, idx);
        for(i=0;i<np;i++){
            grad_plus = *p, grad_minus = *(p+1); 
            *p = 0.5 * (grad_plus + grad_minus);
            *(++p) = 0.5 * (grad_plus - grad_minus); 
            p++;
        } 
    }
}

static void
func_u(FLOAT x, FLOAT y, FLOAT z, FLOAT *value)
{
    *value = R;
}

static void 
func_gradx(FLOAT x, FLOAT y, FLOAT z, FLOAT *value)
{
    *value = x/R;
}

int main(int argc, char *argv[])
{   
    char *meshfile ="../mesh/hollowed_icosahedron.mesh";
    GRID *g; 
    DOF_TYPE *dg_type;
    INT p_order = 2, refine_time = 0;

    //command line options
    phgOptionsRegisterInt("p", "polynomial order of DG basis, default is 2", &p_order);
    phgOptionsRegisterString("m", "name of the mesh file, default is \"cube.dat\"", 
            &meshfile);
    phgOptionsRegisterInt("r", "mesh refine times, default is 0", &refine_time);

    phgInit(&argc, &argv);	
    switch(p_order){
        case 0: dg_type = DOF_DG0;
            break;
        case 1: dg_type = DOF_DG1;
            break;
        case 2: dg_type = DOF_DG2; 
            break;                
        case 3: dg_type = DOF_DG3;
            break;
        case 4: dg_type = DOF_DG4;
            break;                
        case 5: dg_type = DOF_DG5;
            break;
        case 6: dg_type = DOF_DG6;
            break;                
        case 7: dg_type = DOF_DG7;
            break;
        case 8: dg_type = DOF_DG8;
            break;                
        case 9: dg_type = DOF_DG9;
            break;
        case 10: dg_type = DOF_DG10;
            break;
        case 11: dg_type = DOF_DG11;
            break;
        case 12: dg_type = DOF_DG12;
            break;
        case 13: dg_type = DOF_DG13;
            break;
        case 14: dg_type = DOF_DG14;
            break;
        case 15: dg_type = DOF_DG15;
            break;
        default: dg_type = DOF_DG2;
            phgPrintf("\nUnavailable polynomial order, using default set DOF_DG2\n");
    }

    g = phgNewGrid(-1); 
    phgImport(g, meshfile, FALSE);
    phgBalanceGrid(g, 1.2, 1, NULL, 0.);
    phgRefineAllElements(g, refine_time);

    phgPrintf("\nUsing mesh file: %s\n", meshfile);
    phgPrintf("Refine times: %d\n", refine_time);
    phgPrintf("Highest polymonial order: %d\n", dg_type->order);
    phgPrintf("Total elements = %d\n", g->nelem_global);
    phgPrintf("Total processes = %d\n\n", phgNProcs);

    DOF *u, *gradz_numerical, *gradz_ave, *gradz_diff, *gradz_analytic, *gradz_err;
    u = phgDofNew(g, dg_type, 1, "u", DofInterpolation);
    gradz_numerical = phgDofNew(g, dg_type, 2, "numerical", DofInterpolation);
    gradz_ave = phgDofNew(g, dg_type, 1, "ave", DofInterpolation);
    gradz_diff = phgDofNew(g, dg_type, 1, "diff", DofInterpolation);
    gradz_err = phgDofNew(g, dg_type, 1, "err", DofInterpolation);
    gradz_analytic = phgDofNew(g, dg_type, 1, "analytic", DofNoAction);
    phgDofSetDataByFunction(u, func_Psi33);
    phgDofSetDataByFunction(gradz_analytic, func_Phi333);

    dgHJGradDof(u, u, gradz_numerical, 2);
    get_dof_grad_hat(gradz_numerical);
    
    split_dof(gradz_numerical, gradz_ave, gradz_diff);
    //phgDofDump(gradz_ave);
    phgDofCopy(gradz_ave, &gradz_err, NULL, "err");
    phgDofAXPBY(1.0, gradz_analytic, -1.0, &gradz_err);
    phgPrintf("The L2 error of gradz = %.16lf\n", phgDofNormL2(gradz_err));
    
    phgDofFree(&u);
    phgDofFree(&gradz_numerical);
    phgDofFree(&gradz_ave);
    phgDofFree(&gradz_diff);
    phgDofFree(&gradz_err);
    phgDofFree(&gradz_analytic);

    phgFinalize();
    return 0;
}
