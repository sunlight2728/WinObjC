//******************************************************************************
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//******************************************************************************

#pragma once

#include "ShaderInfo.h"
#include <set>

struct ShaderNode;
typedef map<string, ShaderNode*> ShaderDef;
typedef set<string> StrSet;
typedef vector<ShaderNode*> ShaderNodes;

struct TempInfo {
    inline TempInfo() : type(SVT_INVALID) {}
    inline TempInfo(ShaderVarType type, const string& body) : type(type), body(body) {}

    bool dependsOn(const StrSet& set) const;
    
    ShaderVarType type;
    string body;
};
typedef map<string, TempInfo> TempMap;

@class GLKShaderPair;

class ShaderContext {
    ShaderLayout            shaderVars;

    ShaderDef               vs;
    ShaderDef               ps;

    bool                    vertexStage;
    TempMap                 vsTemps, vsTempVals;
    TempMap                 psTemps, psTempVals;
    
protected:
    string orderedTempVals(const TempMap& temps);
    
    string generate(ShaderLayout& outputs, ShaderLayout& inputs, const ShaderDef& shader,
                    const string& desc, ShaderLayout* usedOutputs = NULL);

public:
    ShaderContext(const ShaderDef& vert, const ShaderDef& pixel) :
        vs(vert), ps(pixel), vertexStage(false) {}

    // NOTE: neither of these check for interdependencies or overwriting.
    void addTempFunc(ShaderVarType type, const string& name, const string& body);
    void addTempVal(ShaderVarType type, const string& name, const string& body);

    GLKShaderPair* generate(ShaderLayout& inputs);
};

// --------------------------------------------------------------------------------

struct ShaderNode {
    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) { return false; }
};

class ShaderVarRef : public ShaderNode {    
    string name;
    string constantResult;
public:
    ShaderVarRef(const string& name, const string& constantResult = "") : name(name), constantResult(constantResult) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderFallbackRef : public ShaderNode {
    string first;
    string second;
    string constantResult;
public:
    ShaderFallbackRef(const string& first, const string& second, const string& constantResult = "") :
        first(first), second(second), constantResult(constantResult) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

struct ShaderPosRef : public ShaderNode {
    inline ShaderPosRef() {}
    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderTexRef : public ShaderNode {
    string texVar;
    ShaderNode* uvRef;
    ShaderNode* nextRef;

public:
    ShaderTexRef(const string& tex, ShaderNode* uvRef, ShaderNode* nextRef) :
        texVar(tex), uvRef(uvRef), nextRef(nextRef) {}
    
    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderAdditiveCombiner : public ShaderNode {
    ShaderNodes subNodes;

public:
    inline ShaderAdditiveCombiner() {}
    inline ShaderAdditiveCombiner(const ShaderNodes& n) : subNodes(n) {}

    inline void addNode(ShaderNode* n) { subNodes.push_back(n); }
    
    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;    
};

class ShaderOp : public ShaderNode {
    ShaderNode* n1;
    ShaderNode* n2;
    string op;
    bool isOperator;
    bool needsAll;

public:
    inline ShaderOp(ShaderNode* n1, ShaderNode* n2, const string& op, bool isOperator, bool needsAll = false) :
        n1(n1), n2(n2), op(op), isOperator(isOperator), needsAll(needsAll) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

// Used to save stuff into a temp.  Only valuable if reused > 1 time.
class ShaderTempRef : public ShaderNode {
    ShaderVarType type;
    string name;
    ShaderNode* body;

public:
    inline ShaderTempRef(ShaderVarType type, const string& name, ShaderNode* n) :
        type(type), name(name), body(n) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderAttenuator : public ShaderNode {
    ShaderNode* toLight;
    ShaderNode* atten;
public:
    inline ShaderAttenuator(ShaderNode* toLight, ShaderNode* atten) :
        toLight(toLight), atten(atten) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderLighter : public ShaderNode {
    ShaderNode* lightDir;
    ShaderNode* normal;
    ShaderNode* color;
    ShaderNode* atten;

public:
    inline ShaderLighter(ShaderNode* lightDir, ShaderNode* normal, ShaderNode* color, ShaderNode* atten) :
        lightDir(lightDir), normal(normal), color(color), atten(atten) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};

class ShaderSpecLighter : public ShaderNode {
    ShaderNode* lightDir;
    ShaderNode* cameraDir;
    ShaderNode* normal;
    ShaderNode* color;
    ShaderNode* atten;

public:
    inline ShaderSpecLighter(ShaderNode* lightDir, ShaderNode* cameraDir, ShaderNode* normal,
                             ShaderNode* color, ShaderNode* atten) :
        lightDir(lightDir), cameraDir(cameraDir), normal(normal), color(color), atten(atten) {}

    virtual bool generate(string& out, ShaderContext& c, ShaderLayout& v) override;
};
