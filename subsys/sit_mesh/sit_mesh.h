/*
 *  sit_mesh.h
 */
#ifndef __SIT_MESH_H__
#define __SIT_MESH_H__

#ifdef __cplusplus
extern "C"
{
#endif


void sit_mesh_init(void);
void sit_mesh_prov_init(void);

bool sit_mesh_is_provisioned(void);



#ifdef __cplusplus
}
#endif


#endif //__SIT_MESH_H__