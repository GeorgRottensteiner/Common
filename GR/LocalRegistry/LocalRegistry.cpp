#include "LocalRegistry.h"

#include <Misc/Format.h>

#include <String/Convert.h>
#include <String/XML.h>

#include <IO/FileStream.h>

LocalRegistry::Variable::Variable(LocalRegistry& lr, const GR::String& Name, const GR::String& Value) :
m_Name(Name),
m_pRegistry(&lr),
m_Temporary(false) {
    lr.SetVar(m_Name, Value);
}

LocalRegistry::Variable& LocalRegistry::Variable::operator=(const GR::String &Input) {
    m_pRegistry->SetVar(m_Name, Input);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator+=(const GR::String &Input) {
    m_pRegistry->SetVar(m_Name, m_pRegistry->GetVar(m_Name) + Input);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator=(const float Input) {
    m_pRegistry->SetVar(m_Name, CMisc::printf("%f", Input));
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator+=(const float Input) {
    GR::String Value = CMisc::printf("%f", (float) atof(m_pRegistry->GetVar(m_Name).c_str()) + Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator-=(const float Input) {
    GR::String Value = CMisc::printf("%f", (float) atof(m_pRegistry->GetVar(m_Name).c_str()) - Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator*=(const float Input) {
    GR::String Value = CMisc::printf("%f", (float) atof(m_pRegistry->GetVar(m_Name).c_str()) * Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator/=(const float Input) {
    GR::String Value = CMisc::printf("%f", (float) atof(m_pRegistry->GetVar(m_Name).c_str()) / Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator=(const int& Input) {
    m_pRegistry->SetVar(m_Name, CMisc::printf("%d", Input));
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator+=(const int& Input) {
    GR::String Value = CMisc::printf("%d", atoi(m_pRegistry->GetVar(m_Name).c_str()) + Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator-=(const int& Input) {
    GR::String Value = CMisc::printf("%d", atoi(m_pRegistry->GetVar(m_Name).c_str()) - Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator*=(const int& Input) {
    GR::String Value = CMisc::printf("%d", atoi(m_pRegistry->GetVar(m_Name).c_str()) * Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator/=(const int& Input) {
    GR::String Value = CMisc::printf("%d", atoi(m_pRegistry->GetVar(m_Name).c_str()) / Input);
    m_pRegistry->SetVar(m_Name, Value);
    return *this;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator++() //- pre-inc
{
    m_pRegistry->SetVarI(m_Name, atoi(m_pRegistry->GetVar(m_Name).c_str()) + 1);
    return *this;
}

LocalRegistry::Variable LocalRegistry::Variable::operator++(int) //- post-inc
{
    LocalRegistry::Variable tmp(*this);

    tmp.m_Temporary = true;
    tmp.m_TempValue = m_pRegistry->GetVar(m_Name);

    m_pRegistry->SetVarI(m_Name, atoi(m_pRegistry->GetVar(m_Name).c_str()) + 1);
    return tmp;
}

LocalRegistry::Variable& LocalRegistry::Variable::operator--() //- pre-inc
{
    m_pRegistry->SetVarI(m_Name, atoi(m_pRegistry->GetVar(m_Name).c_str()) - 1);
    return *this;
}

LocalRegistry::Variable LocalRegistry::Variable::operator--(int) //- post-inc
{
    LocalRegistry::Variable tmp(*this);

    tmp.m_Temporary = true;
    tmp.m_TempValue = m_pRegistry->GetVar(m_Name);

    m_pRegistry->SetVarI(m_Name, atoi(m_pRegistry->GetVar(m_Name).c_str()) - 1);
    return tmp;
}

GR::String LocalRegistry::Variable::ToString() const {
    return m_pRegistry->GetVar(m_Name);
}

LocalRegistry::Variable::operator int() {
    return atoi(m_pRegistry->GetVar(m_Name).c_str());
}

LocalRegistry::Variable::operator float() {
    return (float) atof(m_pRegistry->GetVar(m_Name).c_str());
}

LocalRegistry::Variable::operator GR::String() {
    return m_pRegistry->GetVar(m_Name);
}

LocalRegistry::LocalRegistry() {

}

bool LocalRegistry::SaveVars(const GR::Char* szFileName) const {
    GR::IO::FileStream aFile;

    if (!aFile.Open(szFileName, IIOStream::OT_WRITE_ONLY)) {
        return false;
    }

    aFile.WriteU32((GR::u32)m_Vars.size());

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        const GR::String& strVarName = it->first;
        const GR::String& strVarValue = it->second;

        aFile.WriteU32((GR::u32)strVarName.length());
        aFile.WriteBlock(strVarName.c_str(), (GR::u32)strVarName.length());

        aFile.WriteU32((GR::u32)strVarValue.length());
        aFile.WriteBlock(strVarValue.c_str(), (GR::u32)strVarValue.length());

        it++;
    }
    aFile.Close();

    return true;
}

bool LocalRegistry::SaveVars(const GR::String& FileName) const {
    GR::IO::FileStream aFile;

    if (!aFile.Open(FileName, IIOStream::OT_WRITE_ONLY)) {
        return false;
    }

    aFile.WriteU32((GR::u32)m_Vars.size());

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        const GR::String& strVarName = it->first;
        const GR::String& strVarValue = it->second;

        aFile.WriteU32((GR::u32)strVarName.length());
        aFile.WriteBlock(strVarName.c_str(), (GR::u32)strVarName.length());

        aFile.WriteU32((GR::u32)strVarValue.length());
        aFile.WriteBlock(strVarValue.c_str(), (GR::u32)strVarValue.length());

        it++;
    }
    aFile.Close();

    return true;
}

bool LocalRegistry::SaveVarsINI(const GR::Char* szFileName) const {
    GR::IO::FileStream aFile;

    if (!aFile.Open(szFileName, IIOStream::OT_WRITE_ONLY)) {
        return false;
    }

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        aFile.WriteLine(it->first + "=" + it->second);

        it++;
    }
    aFile.Close();
    return true;
}

bool LocalRegistry::SaveVarsINI(const GR::String& FileName) const {
    GR::IO::FileStream aFile;

    if (!aFile.Open(FileName, IIOStream::OT_WRITE_ONLY)) {
        return false;
    }

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        aFile.WriteLine(it->first + "=" + it->second);

        it++;
    }
    aFile.Close();
    return true;
}

bool LocalRegistry::SaveVarsXML(const char* szFileName) const {
    GR::Strings::XML xmlParser;

    GR::Strings::XMLElement* pRegistry = new GR::Strings::XMLElement("Registry");

    xmlParser.InsertChild(pRegistry);

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        GR::String strVarName = it->first;
        const GR::String& strVarValue = it->second;

        size_t iDotPos = -1;

        GR::Strings::XMLElement* pParent = pRegistry;

        while ((iDotPos = strVarName.find('.')) != GR::String::npos) {
            // ein Child!
            GR::String strParentName = strVarName.substr(0, iDotPos);
            if (!strParentName.empty()) {
                GR::Strings::XMLElement* pElement = xmlParser.FindByType(strParentName);
                if (pElement == NULL) {
                    pElement = new GR::Strings::XMLElement(strParentName);

                    pParent->InsertChild(pElement);
                }
                pParent = pElement;
            }
            strVarName = strVarName.substr(iDotPos + 1);
        }
        if (!strVarName.empty()) {
            pParent->InsertChild(new GR::Strings::XMLElement(strVarName, strVarValue));
        }

        it++;
    }

    return xmlParser.Save(szFileName);

}

bool LocalRegistry::SaveVarsXML(const GR::String& FileName) const {
    GR::Strings::XML xmlParser;

    GR::Strings::XMLElement* pRegistry = new GR::Strings::XMLElement("Registry");

    xmlParser.InsertChild(pRegistry);

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        GR::String strVarName = it->first;
        const GR::String& strVarValue = it->second;

        size_t iDotPos = -1;

        GR::Strings::XMLElement* pParent = pRegistry;

        while ((iDotPos = strVarName.find('.')) != GR::String::npos) {
            // ein Child!
            GR::String strParentName = strVarName.substr(0, iDotPos);
            if (!strParentName.empty()) {
                GR::Strings::XMLElement* pElement = xmlParser.FindByType(strParentName);
                if (pElement == NULL) {
                    pElement = new GR::Strings::XMLElement(strParentName);

                    pParent->InsertChild(pElement);
                }
                pParent = pElement;
            }
            strVarName = strVarName.substr(iDotPos + 1);
        }
        if (!strVarName.empty()) {
            pParent->InsertChild(new GR::Strings::XMLElement(strVarName, strVarValue));
        }

        it++;
    }

    return xmlParser.Save(FileName.c_str());
}

bool LocalRegistry::SaveVars(IIOStream& Stream) const {

    Stream.WriteU32((GR::u32)m_Vars.size());

    tMapVars::const_iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        const GR::String& strVarName = it->first;
        const GR::String& strVarValue = it->second;

        Stream.WriteString(strVarName);
        Stream.WriteString(strVarValue);

        it++;
    }

    return true;

}

bool LocalRegistry::LoadVars(const GR::Char* szFileName) {
    GR::IO::FileStream ioIn(szFileName);
    return LoadVars(ioIn);
}

bool LocalRegistry::LoadVars(const GR::String& FileName) {
    GR::IO::FileStream ioIn(FileName);
    return LoadVars(ioIn);
}

bool LocalRegistry::LoadVars(IIOStream& Stream) {
    Clear();

    if (!Stream.IsGood()) {
        return false;
    }

    int iCount = Stream.ReadU32();

    for (int i = 0; i < iCount; i++) {
        GR::String strVarName;
        GR::String strVarValue;

        Stream.ReadString(strVarName);
        Stream.ReadString(strVarValue);

        m_Vars[strVarName] = strVarValue;
    }

    return true;
}

void LocalRegistry::Merge(const LocalRegistry& lrOther, bool bOverwriteExisting) {

    if (&lrOther == this) {
        return;
    }

    tMapVars::const_iterator it(lrOther.m_Vars.begin());
    while (it != lrOther.m_Vars.end()) {
        if ((m_Vars.find(it->first) != m_Vars.end())
                && (!bOverwriteExisting)) {
        } else {
            m_Vars[it->first] = it->second;
        }

        ++it;
    }

}

bool LocalRegistry::LoadVarsINI(const GR::Char* szFileName) {
    GR::IO::FileStream ioIn(szFileName);
    return LoadVarsINI(ioIn);
}

bool LocalRegistry::LoadVarsINI(const GR::String& FileName) {
    GR::IO::FileStream ioIn(FileName);
    return LoadVarsINI(ioIn);
}

bool LocalRegistry::LoadVarsINI(IIOStream& Stream) {
    Clear();

    if (!Stream.IsGood()) {
        return false;
    }

    GR::String line;

    while (Stream.ReadLine(line)) {
        // empty/comment
        if ((line.empty())
                || (line.StartsWith("#"))) {
            continue;
        }

        size_t equPos = line.find('=');
        if (equPos != GR::String::npos) {
            GR::String varName = line.substr(0, equPos);
            GR::String varValue = line.substr(equPos + 1);
            m_Vars[varName] = varValue;
        }
    }
    return true;
}

bool LocalRegistry::LoadVarsXML(const GR::Char* szFileName) {
    GR::Strings::XML xmlParser;

    Clear();

    if (!xmlParser.Load(szFileName)) {
        return false;
    }

    GR::Strings::XMLElement* pRegistry = xmlParser.FindByType("Registry");
    if (pRegistry == NULL) {
        return false;
    }

    GR::Strings::XML::iterator it(pRegistry->FirstChild());
    while (it != GR::Strings::XML::iterator()) {
        GR::Strings::XMLElement* pElement = *it;

        GR::String strVarName = pElement->Type();
        GR::String strVarValue = pElement->GetContent();

        while ((pElement = pElement->Parent())
                && (pElement != pRegistry)) {
            strVarName = pElement->Type() + '.' + strVarName;
        }

        SetVar(strVarName, strVarValue);

        ++it;
    }

    return true;
}

bool LocalRegistry::LoadVarsXML(const GR::String& FileName) {
    GR::Strings::XML xmlParser;

    Clear();

    if (!xmlParser.Load(FileName.c_str())) {
        return false;
    }

    GR::Strings::XMLElement* pRegistry = xmlParser.FindByType("Registry");
    if (pRegistry == NULL) {
        return false;
    }

    GR::Strings::XML::iterator it(pRegistry->FirstChild());
    while (it != GR::Strings::XML::iterator()) {
        GR::Strings::XMLElement* pElement = *it;

        GR::String strVarName = pElement->Type();
        GR::String strVarValue = pElement->GetContent();

        while ((pElement = pElement->Parent())
                && (pElement != pRegistry)) {
            strVarName = pElement->Type() + '.' + strVarName;
        }

        SetVar(strVarName, strVarValue);

        ++it;
    }

    return true;
}

void LocalRegistry::_SetVar(const GR::String& Name, const GR::String& Value) {

    tMapVars::iterator it(m_Vars.find(Name));

    if (it != m_Vars.end()) {
        it->second = Value;
        SendEvent(LRE_VAR_CHANGED, Name);
    } else {
        m_Vars[Name] = Value;
        SendEvent(LRE_VAR_ADDED, Name);
    }

}

void LocalRegistry::SetVar(const GR::String& Name, const GR::String& Value) {

    _SetVar(Name, Value);

}

void LocalRegistry::SetVarF(const GR::String& Name, const float Value) {
    _SetVar(Name, Misc::Format() << Value);
}

void LocalRegistry::SetVarFDelta(const GR::String& Name, const float Value) {
    _SetVar(Name, Misc::Format() << Value + GetVarF(Name));
}

void LocalRegistry::SetVarI(const GR::String& Name, const GR::i32 Value) {
    _SetVar(Name, Misc::Format() << Value);
}

void LocalRegistry::SetVarIDelta(const GR::String& Name, const GR::i32 Value) {
    _SetVar(Name, Misc::Format() << Value + GetVarI(Name));
}

void LocalRegistry::SetVarI64(const GR::String& Name, const GR::i64 Value) {
    _SetVar(Name, Misc::Format() << Value);
}

void LocalRegistry::SetVarI64Delta(const GR::String& Name, const GR::i64 Value) {
    _SetVar(Name, Misc::Format() << Value + GetVarI64(Name));
}

void LocalRegistry::SetVarUP(const GR::String& Name, const GR::up dwValue) {
    _SetVar(Name, GR::Convert::ToString(dwValue));
}

void LocalRegistry::SetVarUPDelta(const GR::String& Name, const GR::ip iValue) {
    GR::up dwValue = GetVarUP(Name);
    dwValue += iValue;

    _SetVar(Name, GR::Convert::ToString(dwValue));
}

GR::up LocalRegistry::GetVarUP(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return 0;
    }
    return GR::Convert::ToUP(it->second);
}

bool LocalRegistry::VarExists(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    return ( it != m_Vars.end());
}

GR::String LocalRegistry::GetVar(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return GR::String();
    }
    return it->second;
}

GR::i32 LocalRegistry::GetVarI(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return 0;
    }
    return GR::Convert::ToI32(it->second);
}

GR::i64 LocalRegistry::GetVarI64(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return 0;
    }
    return GR::Convert::ToI64(it->second);
}

GR::i64 LocalRegistry::GetVarI64(const GR::String& Name, const GR::i64 max) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return 0;
    }
    auto value = GR::Convert::ToI64(it->second);

    if (value > max) {
        value = max;
    }
    return value;
}



float LocalRegistry::GetVarF(const GR::String& Name) const {
    tMapVars::const_iterator it(m_Vars.find(Name));

    if (it == m_Vars.end()) {
        return 0.0f;
    }
    return (float) atof(it->second.c_str());
}

void LocalRegistry::DeleteVar(const GR::String& Name) {
    tMapVars::iterator it(m_Vars.find(Name));

    if (it != m_Vars.end()) {
        m_Vars.erase(it);
        SendEvent(LRE_VAR_REMOVED, Name);
    }
}

bool LocalRegistry::Empty() const {
    return m_Vars.empty();
}

void LocalRegistry::Clear() {
    tMapVars::iterator it(m_Vars.begin());
    while (it != m_Vars.end()) {
        SendEvent(LRE_VAR_REMOVED, it->first);

        ++it;
    }

    m_Vars.clear();
}

void LocalRegistry::AddHandler(tVarEventHandler Handler) {
    tEventHandlers::iterator it(m_EventHandlers.begin());
    while (it != m_EventHandlers.end()) {
        if (*it == Handler) {
            return;
        }
        ++it;
    }
    m_EventHandlers.push_back(Handler);
}

void LocalRegistry::RemoveHandler(tVarEventHandler Handler) {
    m_EventHandlers.remove(Handler);
}

void LocalRegistry::SendEvent(LocalRegistryEvent Event, const GR::String& Var) {
    tEventHandlers::iterator it(m_EventHandlers.begin());
    while (it != m_EventHandlers.end()) {
        tVarEventHandler & Handler(*it);

        if (Handler) {
            Handler(Event, Var);
        }
        ++it;
    }
}