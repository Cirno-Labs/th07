// The file with Unknown1347b00, th06's Stage, from 0x404fe0, as th06's
// stage.cpp.

#include <string.h>

#include "unsorted.h"

// An inline of unsorted.h, probably, whose out-of-line copy the original
// keeps with this file's; defined here so that the other files' calls
// stay calls. unk_ac has to be unk_34[2] plus unk_b4[2] for each byte
// ComputeChecksum sums (0x84 past unk_ac), unk_b0 plus unk_8c[3] unk_9614.
int Unknown626270::ChecksumMismatch()
{
    return this->stats->unk_ac == this->stats->unk_34[2] + this->stats->unk_b4[2] * ((BYTE *)&this->stats->unk_ac - (BYTE *)this->stats->unk_34 + 0x84) &&
                   this->stats->unk_b0 + this->stats->unk_8c[3] == (int)this->unk_9614
               ? FALSE
               : TRUE;
}

Unknown1347b00::Unknown1347b00()
{
    memset(this, 0, sizeof(Unknown1347b00));
    this->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    this->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    this->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    this->camera.fov = 0.5235988f;
    this->unk_50f8 = this->camera;
    this->unk_5138 = this->camera;
}

float HermiteInterpolate(float p0, float p1, float m0, float m1, float t)
{
    float f00;
    float f10;
    float f01;
    float f11;

    f00 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    f01 = t * t * (3.0f - 2.0f * t);
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    return f00 * p0 + f01 * p1 + f10 * m0 + f11 * m1;
}

void UpdateEasedVector(Unknown1347b00 *stage, int i, D3DXVECTOR3 *out, D3DXVECTOR3 *from, D3DXVECTOR3 *to, D3DXVECTOR3 *m0, D3DXVECTOR3 *m1)
{
    float t;

    if (stage->unk_5248[i] < stage->unk_5238[i])
    {
        stage->unk_5248[i].Tick();
        t = stage->unk_5248[i].AsFloat() / stage->unk_5238[i];
    }
    else
    {
        stage->unk_5248[i].Set(stage->unk_5238[i]);
        t = 1.0f;
        stage->unk_5238[i] = 0;
    }
    switch (stage->unk_5278[i])
    {
    case 1:
        t = 1.0f - (t = 1.0f - t) * t;
        break;
    case 2:
        t = 1.0f - (t = 1.0f - t) * t * t;
        break;
    case 3:
        t = 1.0f - (t = 1.0f - t) * t * t * t;
        break;
    case 4:
        t = t * t;
        break;
    case 5:
        t = t * t * t;
        break;
    case 6:
        t = t * t * t * t;
        break;
    }
    if (stage->unk_5278[i] != 7)
    {
        *out = *to - *from;
        *out = t * *out + *from;
    }
    else
    {
        out->x = HermiteInterpolate(from->x, to->x, m0->x, m1->x, t);
        out->y = HermiteInterpolate(from->y, to->y, m0->y, m1->y, t);
        out->z = HermiteInterpolate(from->z, to->z, m0->z, m1->z, t);
    }
}

void Unknown1347b00::SetTintColor(D3DCOLOR color)
{
    ColorBytes c;

    if (this->unk_52ac.a == 0)
    {
        this->unk_52ac.color = color;
    }
    else
    {
        c.color = color;
        this->unk_52ac.r = (DWORD)(c.r + this->unk_52ac.r) >> 1;
        this->unk_52ac.g = (DWORD)(c.g + this->unk_52ac.g) >> 1;
        this->unk_52ac.b = (DWORD)(c.b + this->unk_52ac.b) >> 1;
        this->unk_52ac.a = (DWORD)(c.a + this->unk_52ac.a) >> 1;
    }
}

int Unknown1347b00::DeletedCallback(Unknown1347b00 *stage)
{
    g_SpriteEngine->ReleaseAnm(5);
    g_SpriteEngine->ReleaseAnm(6);
    g_SpriteEngine->ReleaseAnm(7);
    g_SpriteEngine->ReleaseAnm(8);
    g_SpriteEngine->ReleaseAnm(9);
    if (stage->sprites != NULL)
    {
        MemFree(stage->sprites);
        stage->sprites = NULL;
    }
    if (stage->data != NULL)
    {
        MemFree(stage->data);
        stage->data = NULL;
    }
    return 0;
}

int Unknown1347b00::RegisterChain(int stage)
{
    Unknown1347b00 *s = &g_Unknown1347b00;

    memset(s, 0, sizeof(Unknown1347b00));
    s->data = NULL;
    s->unk_4c4 = 0;
    s->stage = stage;
    g_Unknown1347b00CalcChain.SetCallback((ChainCallback)Unknown1347b00::OnUpdate);
    g_Unknown1347b00CalcChain.addedCallback = (ChainCallback)Unknown1347b00::AddedCallback;
    g_Unknown1347b00CalcChain.deletedCallback = (ChainCallback)Unknown1347b00::DeletedCallback;
    g_Unknown1347b00CalcChain.arg = s;
    if (g_Chain.AddToCalcChain(&g_Unknown1347b00CalcChain, 7) != 0)
    {
        return -1;
    }
    g_Unknown1347b00DrawChain.SetCallback((ChainCallback)Unknown1347b00::OnDraw);
    g_Unknown1347b00DrawChain.arg = s;
    g_Chain.AddToDrawChain(&g_Unknown1347b00DrawChain, 3);
    g_Unknown1347b00DrawChain2.SetCallback((ChainCallback)Unknown1347b00::OnDraw2);
    g_Unknown1347b00DrawChain2.arg = s;
    g_Chain.AddToDrawChain(&g_Unknown1347b00DrawChain2, 4);
    return 0;
}
