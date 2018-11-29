#include <string.h>

static void
create_dofs(GRID *g, DOF_TYPE *type, INT dim, DOF **dofs_list, char *name_head, INT ndof)
{
    char name[15], name_idx[5];
    for(INT i=0; i<ndof; i++){
        strcpy(name, name_head);
        sprintf(name_idx, "_%d", i);
        strcat(name, name_idx);
        dofs_list[i] = phgDofNew(g, type, dim, name, DofInterpolation);
    } 
}

static void
list2tensor(FLOAT *list, FLOAT *tensor, int dim)
{
    int i, j;
    FLOAT *p = tensor;
    for(i=0;i<dim;i++){
        *p = *list;
        for(j=1;j<dim-i;j++){
            *(++tensor) = *(++list);
            p += dim;
            *p = *list;
        }
        list++;
        tensor += i+2;
        p -= dim*(dim-i-2) - 1; 
    }
}