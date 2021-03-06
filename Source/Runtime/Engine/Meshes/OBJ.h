// ===========================================================================
// 
// HevEn (C) 2017 by Hevedy <https://github.com/Hevedy>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, 
// You can obtain one at https://mozilla.org/MPL/2.0/.
// 
// ---------------------------------------------------------------------------
// 
// Sauerbraten game engine source code, any release.
// Tesseract game engine source code, any release.
// 
// Copyright (C) 2001-2017 Wouter van Oortmerssen, Lee Salzman, Mike Dysart, Robert Pointon, and Quinton Reeves
// Copyright (C) 2001-2017 Wouter van Oortmerssen, Lee Salzman, Mike Dysart, Robert Pointon, Quinton Reeves, and Benjamin Segovia
// 
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// 
// ---------------------------------------------------------------------------
//  File:			OBJ.h
//  Description: 	OBJ models header
// ---------------------------------------------------------------------------
//  Log:			Source.
//
//
// ===========================================================================


struct obj;

struct obj : vertmodel, vertloader<obj>
{
    obj(const char *name) : vertmodel(name) {}

    static const char *formatname() { return "obj"; }
    static bool cananimate() { return false; }
    int type() const { return MDL_OBJ; }

    struct objmeshgroup : vertmeshgroup
    {
        void parsevert(char *s, vector<vec> &out)
        {
            vec &v = out.add(vec(0, 0, 0));
            while(isalpha(*s)) s++;
            loopi(3)
            {
                v[i] = strtod(s, &s);
                while(isspace(*s)) s++;
                if(!*s) break;
            }
        }

        bool load(const char *filename, float smooth)
        {
            int len = strlen(filename);
            if(len < 4 || strcasecmp(&filename[len-4], ".obj")) return false;

            stream *file = openfile(filename, "rb");
            if(!file) return false;

            name = newstring(filename);

            numframes = 1;

            vector<vec> attrib[3];
            char buf[512];

            hashtable<ivec, int> verthash(1<<11);
            vector<vert> verts;
            vector<tcvert> tcverts;
            vector<tri> tris;

            #define STARTMESH do { \
                vertmesh &m = *new vertmesh; \
                m.group = this; \
                m.name = meshname[0] ? newstring(meshname) : NULL; \
                meshes.add(&m); \
                curmesh = &m; \
                verthash.clear(); \
                verts.setsize(0); \
                tcverts.setsize(0); \
                tris.setsize(0); \
            } while(0)

            #define FLUSHMESH do { \
                curmesh->numverts = verts.length(); \
                if(verts.length()) \
                { \
                    curmesh->verts = new vert[verts.length()]; \
                    memcpy(curmesh->verts, verts.getbuf(), verts.length()*sizeof(vert)); \
                    curmesh->tcverts = new tcvert[verts.length()]; \
                    memcpy(curmesh->tcverts, tcverts.getbuf(), tcverts.length()*sizeof(tcvert)); \
                } \
                curmesh->numtris = tris.length(); \
                if(tris.length()) \
                { \
                    curmesh->tris = new tri[tris.length()]; \
                    memcpy(curmesh->tris, tris.getbuf(), tris.length()*sizeof(tri)); \
                } \
                if(attrib[2].empty()) \
                { \
                    if(smooth <= 1) curmesh->smoothnorms(smooth); \
                    else curmesh->buildnorms(); \
                } \
                curmesh->calctangents(); \
            } while(0)

            string meshname = "";
            vertmesh *curmesh = NULL;
            while(file->getline(buf, sizeof(buf)))
            {
                char *c = buf;
                while(isspace(*c)) c++;
                switch(*c)
                {
                    case '#': continue;
                    case 'v':
                        if(isspace(c[1])) parsevert(c, attrib[0]);
                        else if(c[1]=='t') parsevert(c, attrib[1]);
                        else if(c[1]=='n') parsevert(c, attrib[2]);
                        break;
                    case 'g':
                    {
                        while(isalpha(*c)) c++;
                        while(isspace(*c)) c++;
                        char *name = c;
                        size_t namelen = strlen(name);
                        while(namelen > 0 && isspace(name[namelen-1])) namelen--;
                        copystring(meshname, name, min(namelen+1, sizeof(meshname)));

                        if(curmesh) FLUSHMESH;
                        curmesh = NULL;
                        break;
                    }
                    case 'f':
                    {
                        if(!curmesh) STARTMESH;
                        int v0 = -1, v1 = -1;
                        while(isalpha(*c)) c++;
                        for(;;)
                        {
                            while(isspace(*c)) c++;
                            if(!*c) break;
                            ivec vkey(-1, -1, -1);
                            loopi(3)
                            {
                                vkey[i] = strtol(c, &c, 10);
                                if(vkey[i] < 0) vkey[i] = attrib[i].length() + vkey[i];
                                else vkey[i]--;
                                if(!attrib[i].inrange(vkey[i])) vkey[i] = -1;
                                if(*c!='/') break;
                                c++;
                            }
                            int *index = verthash.access(vkey);
                            if(!index)
                            {
                                index = &verthash[vkey];
                                *index = verts.length();
                                vert &v = verts.add();
                                v.pos = vkey.x < 0 ? vec(0, 0, 0) : attrib[0][vkey.x];
                                v.pos = vec(v.pos.z, -v.pos.x, v.pos.y);
                                v.norm = vkey.z < 0 ? vec(0, 0, 0) : attrib[2][vkey.z];
                                v.norm = vec(v.norm.z, -v.norm.x, v.norm.y);
                                tcvert &tcv = tcverts.add();
                                tcv.tc = vkey.y < 0 ? vec2(0, 0) : vec2(attrib[1][vkey.y].x, 1-attrib[1][vkey.y].y);
                            }
                            if(v0 < 0) v0 = *index;
                            else if(v1 < 0) v1 = *index;
                            else
                            {
                                tri &t = tris.add();
                                t.vert[0] = ushort(*index);
                                t.vert[1] = ushort(v1);
                                t.vert[2] = ushort(v0);
                                v1 = *index;
                            }
                        }
                        break;
                    }
                }
            }

            if(curmesh) FLUSHMESH;

            delete file;

            return true;
        }
    };

    vertmeshgroup *newmeshes() { return new objmeshgroup; }

    bool loaddefaultparts()
    {
        part &mdl = addpart();
        const char *pname = parentdir(name);
        defformatstring(name1, "Game/Data/Meshes/%s/tris.obj", name);
        mdl.meshes = sharemeshes(path(name1));
        if(!mdl.meshes)
        {
            defformatstring(name2, "Game/Data/Meshes/%s/tris.obj", pname);    // try obj in parent folder (vert sharing)
            mdl.meshes = sharemeshes(path(name2));
            if(!mdl.meshes) return false;
        }
        Texture *tex, *masks;
        loadskin(name, pname, tex, masks);
        mdl.initskins(tex, masks);
        if(tex==notexture) conoutf("could not load model skin for %s", name1);
        return true;
    }

    bool load()
    {
        formatstring(dir, "Game/Data/Meshes/%s", name);
        defformatstring(cfgname, "Game/Data/Meshes/%s/DefOBJ.hed", name);

        loading = this;
        identflags &= ~IDF_PERSIST;
        if(execfile(cfgname, false) && parts.length()) // configured obj, will call the obj* commands below
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            loopv(parts) if(!parts[i]->meshes) return false;
        }
        else // obj without configuration, try default tris and skin
        {
            identflags |= IDF_PERSIST;
            loading = NULL;
            if(!loaddefaultparts()) return false;
        }
        translate.y = -translate.y;
        loaded();
        return true;
    }
};

vertcommands<obj> objcommands;

