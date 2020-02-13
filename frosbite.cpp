#include "frosbite.h"

bool fb::ClientSoldierEntity::GetBonePos(int BoneId, Vec3* vOut)
{
    BoneCollisionComponent* pBoneCollisionComponent = this->bonecollisioncomponent;

    if (!ValidPointer(pBoneCollisionComponent)) 
        return false;

    fb::QuatTransform* pQuat = pBoneCollisionComponent->m_ragdollTransforms.m_ActiveWorldTransforms;

    if (!ValidPointer(pQuat)) 
        return false;

    vOut->x = pQuat[BoneId].m_TransAndScale.x;
    vOut->y = pQuat[BoneId].m_TransAndScale.y;
    vOut->z = pQuat[BoneId].m_TransAndScale.z;

    return true;
}