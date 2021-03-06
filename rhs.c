#include "phg.h"
#include "auxi_dofs.h"
#include "Hhat.h"
#include "global_def.h"
#include "hdw.h"

void
get_dofs_rhs(DOF **dofs_var, DOF **dofs_bdry_in, DOF **dofs_bdry_out, DOF **dofs_g, DOF **dofs_N, DOF **dofs_H, DOF **dofs_deriH, DOF **dofs_src, DOF **dofs_C,
        DOF **dofs_gradPsi, DOF **dofs_gradPi, DOF **dofs_gradPhi, DOF **dofs_Hhat, DOF **dofs_rhs)
{
    short i;
    get_dofs_g(dofs_var, dofs_g); 
    get_dofs_N(dofs_var, dofs_g, dofs_N);
    get_dofs_src_and_C(dofs_var, dofs_g, dofs_N, dofs_H, dofs_deriH, dofs_src, dofs_C); 
    //phgExportVTKn(dofs_src[0]->g, "src.vtk", NVAR, dofs_src);
    //phgExportVTKn(dofs_g[0]->g, "g.vtk", 6, dofs_g);
    //phgExportVTKn(dofs_N[0]->g, "N.vtk", 4, dofs_N);

    get_dofs_Hhat(dofs_var, dofs_bdry_in, dofs_bdry_out, dofs_g, dofs_N,
            dofs_gradPsi, dofs_gradPi, dofs_gradPhi, dofs_Hhat);

    //phgDofDump(dofs_gradPsi[0]);
    //phgExportVTKn(dofs_gradPsi[0]->g, "gradPsi.vtk", 30, dofs_gradPsi);
    //phgExportVTKn(dofs_gradPsi[0]->g, "gradPi.vtk", 30, dofs_gradPi);
    //phgExportVTKn(dofs_gradPsi[0]->g, "gradxPhi.vtk", 30, dofs_gradPhi);
    //phgExportVTKn(dofs_gradPsi[0]->g, "gradyPhi.vtk", 30, dofs_gradPhi + 30);
    //phgExportVTKn(dofs_gradPsi[0]->g, "gradzPhi.vtk", 30, dofs_gradPhi + 60);

    copy_dofs(dofs_src, dofs_rhs, "rhs", NVAR);
    for(i=0;i<NVAR;i++){
        phgDofAXPBY(-1.0, dofs_Hhat[i], 1.0, dofs_rhs + i);
    }
}
