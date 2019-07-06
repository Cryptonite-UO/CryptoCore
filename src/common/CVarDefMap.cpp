
#include "../common/CLog.h"
#include "../game/CServer.h"
#include "CException.h"
#include "CExpression.h"
#include "CScript.h"
#include "CTextConsole.h"
#include "CVarDefMap.h"


inline static int VarDefCompare(const CVarDefCont* pVar, lpctstr ptcKey)
{
    return strcmpi(pVar->GetKey(), ptcKey);
}

/***************************************************************************
*
*
*	class CVarDefContNum		Variable implementation (Number)
*
*
***************************************************************************/

CVarDefContNum::CVarDefContNum( lpctstr pszKey, int64 iVal ) : m_sKey( pszKey ), m_iVal( iVal )
{
}

CVarDefContNum::CVarDefContNum( lpctstr pszKey ) : m_sKey( pszKey ), m_iVal( 0 )
{
}

lpctstr CVarDefContNum::GetValStr() const
{
    return Str_FromLL(m_iVal, Str_GetTemp(), 16);
}

bool CVarDefContNum::r_LoadVal( CScript & s )
{
	SetValNum( s.GetArg64Val() );
	return true;
}

bool CVarDefContNum::r_WriteVal( lpctstr pKey, CSString & sVal, CTextConsole * pSrc )
{
	UNREFERENCED_PARAMETER(pKey);
	UNREFERENCED_PARAMETER(pSrc);
	sVal.Format64Val( GetValNum() );
	return true;
}

CVarDefCont * CVarDefContNum::CopySelf() const
{ 
	return new CVarDefContNum( GetKey(), m_iVal );
}

/***************************************************************************
*
*
*	class CVarDefContStr		Variable implementation (String)
*
*
***************************************************************************/

CVarDefContStr::CVarDefContStr( lpctstr pszKey, lpctstr pszVal ) : m_sKey( pszKey ), m_sVal( pszVal ) 
{
}

CVarDefContStr::CVarDefContStr( lpctstr pszKey ) : m_sKey( pszKey )
{
}

int64 CVarDefContStr::GetValNum() const
{
	lpctstr pszStr = m_sVal;
	return( Exp_Get64Val(pszStr) );
}

void CVarDefContStr::SetValStr( lpctstr pszVal ) 
{
    size_t uiLen = strlen(pszVal);
	if (uiLen <= SCRIPT_MAX_LINE_LEN/2)
		m_sVal.CopyLen( pszVal, (int)uiLen );
	else
		g_Log.EventWarn("Setting max length of %d was exceeded on (VAR,TAG,LOCAL).%s \r", SCRIPT_MAX_LINE_LEN/2, GetKey() );
}

bool CVarDefContStr::r_LoadVal( CScript & s )
{
	SetValStr( s.GetArgStr());
	return true;
}

bool CVarDefContStr::r_WriteVal( lpctstr pKey, CSString & sVal, CTextConsole * pSrc )
{
	UNREFERENCED_PARAMETER(pKey);
	UNREFERENCED_PARAMETER(pSrc);
	sVal = GetValStr();
	return true;
}

CVarDefCont * CVarDefContStr::CopySelf() const 
{ 
	return new CVarDefContStr( GetKey(), m_sVal ); 
}


/***************************************************************************
*
*
*	class CVarDefMap			Holds list of pairs KEY = VALUE and operates it
*
*
***************************************************************************/

CVarDefMap & CVarDefMap::operator = ( const CVarDefMap & array )
{
	Copy( &array );
	return( *this );
}

CVarDefMap::~CVarDefMap()
{
	Empty();
}

lpctstr CVarDefMap::FindValStr( lpctstr pVal ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::FindValStr");
	for ( const CVarDefCont * pVarBase : m_Container )
	{
		ASSERT( pVarBase );
		
		const CVarDefContStr * pVarStr = dynamic_cast <const CVarDefContStr *>( pVarBase );
		if ( pVarStr == nullptr )
			continue;
		
		if ( ! strcmpi( pVal, pVarStr->GetValStr()))
			return pVarBase->GetKey();
	}

	return nullptr;
}

lpctstr CVarDefMap::FindValNum( int64 iVal ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::FindValNum");
    for (const CVarDefCont* pVarBase : m_Container)
	{
		ASSERT( pVarBase );
		const CVarDefContNum * pVarNum = dynamic_cast <const CVarDefContNum *>( pVarBase );
		if ( pVarNum == nullptr )
			continue;

		if ( pVarNum->GetValNum() == iVal )
			return pVarBase->GetKey();
	}

	return nullptr;
}

CVarDefCont * CVarDefMap::GetAt( size_t at ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetAt");
	if ( at > m_Container.size() )
		return nullptr;
    return m_Container[at];
}

CVarDefCont * CVarDefMap::GetAtKey( lpctstr ptcKey ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetAtKey");
    size_t idx = m_Container.find_predicate(ptcKey, VarDefCompare);

	if ( idx != SCONT_BADINDEX )
		return m_Container[idx];
	return nullptr;
}

void CVarDefMap::DeleteAt( size_t at )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::DeleteAt");
	if ( at > m_Container.size() )
		return;

    CVarDefCont *pVarBase = m_Container[at];
    m_Container.erase(m_Container.begin() + at);

    if ( pVarBase )
    {
        CVarDefContNum *pVarNum = dynamic_cast<CVarDefContNum *>(pVarBase);
        if ( pVarNum )
        {
            delete pVarNum;
        }
        else
        {
            CVarDefContStr *pVarStr = dynamic_cast<CVarDefContStr *>(pVarBase);
            if ( pVarStr )
                delete pVarStr;
        }
    }
}

void CVarDefMap::DeleteAtKey( lpctstr ptcKey )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::DeleteAtKey");
    size_t idx = m_Container.find_predicate(ptcKey, VarDefCompare);
    if (idx != SCONT_BADINDEX)
        DeleteAt(idx);
}

void CVarDefMap::DeleteKey( lpctstr key )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::DeleteKey");
	if ( key && *key)
		DeleteAtKey(key);
}

void CVarDefMap::Empty()
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::Empty");
	iterator it = m_Container.begin();
	while ( it != m_Container.end() )
	{
		delete (*it);	// This calls the appropriate destructors, from derived to base class, because the destructors are virtual.
		m_Container.erase(it);
		it = m_Container.begin();
	}

	m_Container.clear();
}

void CVarDefMap::Copy( const CVarDefMap * pArray )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::Copy");
	if ( !pArray || pArray == this )
		return;

	Empty();
	if ( pArray->GetCount() <= 0 )
		return;

    for (const CVarDefCont* pVar : pArray->m_Container)
	{
		m_Container.insert( pVar->CopySelf() );
	}
}

bool CVarDefMap::Compare( const CVarDefMap * pArray )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::Compare");
	if ( !pArray )
		return false;
	if ( pArray == this )
		return true;
	if ( pArray->GetCount() != GetCount() )
		return false;

	if (pArray->GetCount())
	{
        for (const CVarDefCont* pVar : pArray->m_Container)
		{
			lpctstr sKey = pVar->GetKey();
			if (!GetKey(sKey))
				return false;

			if (strcmpi(GetKeyStr(sKey),pVar->GetValStr()))
				return false;
		}
	}
	return true;
}

bool CVarDefMap::CompareAll( const CVarDefMap * pArray )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::CompareAll");
	if ( !pArray )
		return false;
	if ( pArray == this )
		return true;

	if (pArray->GetCount())
	{
        for (const CVarDefCont* pVar : pArray->m_Container)
		{
			lpctstr sKey = pVar->GetKey();

			if (strcmpi(GetKeyStr(sKey, true),pVar->GetValStr()))
				return false;
		}
	}
	if (GetCount())
	{
        for (const CVarDefCont* pVar : m_Container)
		{
			lpctstr sKey = pVar->GetKey();

			if (strcmpi(pArray->GetKeyStr(sKey, true),pVar->GetValStr()))
				return false;
		}
	}
	return true;
}

size_t CVarDefMap::GetCount() const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetCount");
	return m_Container.size();
}

CVarDefContNum* CVarDefMap::SetNumNew( lpctstr pszName, int64 iVal )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetNumNew");
	CVarDefContNum * pVarNum = new CVarDefContNum( pszName, iVal );
	if ( !pVarNum )
		return nullptr;

	iterator res = m_Container.emplace(static_cast<CVarDefCont*>(pVarNum));
	if ( res != m_Container.end() )
		return pVarNum;
	else
    {
        delete pVarNum;
		return nullptr;
    }
}

CVarDefContNum* CVarDefMap::SetNumOverride( lpctstr pszKey, int64 iVal )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetNumOverride");
    CVarDefContNum* pKeyNum = dynamic_cast<CVarDefContNum*>(GetKey(pszKey));
    if (pKeyNum)
    {
        pKeyNum->SetValNum(iVal);
        return pKeyNum;
    }
	DeleteAtKey(pszKey);
	return SetNumNew(pszKey,iVal);
}

CVarDefContNum* CVarDefMap::ModNum(lpctstr pszName, int64 iMod, bool fDeleteZero)
{
    ADDTOCALLSTACK_INTENSIVE("CVarDefMap::ModNum");
    ASSERT(pszName);
    CVarDefCont* pVarDef = GetKey(pszName);
    if (pVarDef)
    {
        CVarDefContNum* pVarDefNum = dynamic_cast<CVarDefContNum*>(pVarDef);
        if (pVarDefNum)
        {
            const int64 iNewVal = pVarDefNum->GetValNum() + iMod;
            if ((iNewVal == 0) && fDeleteZero)
            {
                size_t idx = m_Container.find(pVarDef);
                ASSERT (idx != SCONT_BADINDEX);
                DeleteAt(idx);
                return nullptr;
            }
            pVarDefNum->SetValNum(iNewVal);
            return pVarDefNum;
        }
    }
    return SetNum(pszName, iMod, fDeleteZero);
}

CVarDefContNum* CVarDefMap::SetNum( lpctstr pszName, int64 iVal, bool fDeleteZero, bool fWarnOverwrite )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetNum");
	ASSERT(pszName);

	if ( pszName[0] == '\0' )
		return nullptr;

	if ( (iVal == 0) && fDeleteZero )
	{
		DeleteAtKey(pszName);
		return nullptr;
	}

    size_t idx = m_Container.find_predicate(pszName, VarDefCompare);

	CVarDefCont * pVarBase = nullptr;
	if ( idx != SCONT_BADINDEX )
		pVarBase = m_Container[idx];

	if ( !pVarBase )
		return SetNumNew( pszName, iVal );

	CVarDefContNum * pVarNum = dynamic_cast <CVarDefContNum *>( pVarBase );
	if ( pVarNum )
    {
        if ( fWarnOverwrite && !g_Serv.IsResyncing() && g_Serv.IsLoading() )
            DEBUG_WARN(( "Replacing existing VarNum '%s' with number: 0x%" PRIx64" \n", pVarBase->GetKey(), iVal ));
		pVarNum->SetValNum( iVal );
    }
	else
	{
		if ( fWarnOverwrite && !g_Serv.IsResyncing() && g_Serv.IsLoading() )
			DEBUG_WARN(( "Replacing existing VarStr '%s' with number: 0x%" PRIx64" \n", pVarBase->GetKey(), iVal ));
		return SetNumOverride( pszName, iVal );
	}

	return pVarNum;
}

CVarDefContStr* CVarDefMap::SetStrNew( lpctstr pszName, lpctstr pszVal )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetStrNew");
	CVarDefContStr * pVarStr = new CVarDefContStr( pszName, pszVal );
	if ( !pVarStr )
		return nullptr;

    iterator res = m_Container.emplace(static_cast<CVarDefCont*>(pVarStr));
    if ( res != m_Container.end() )
		return pVarStr;
	else
    {
        delete pVarStr;
		return nullptr;
    }
}

CVarDefContStr* CVarDefMap::SetStrOverride( lpctstr pszKey, lpctstr pszVal )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetStrOverride");
    CVarDefContStr* pKeyStr = dynamic_cast<CVarDefContStr*>(GetKey(pszKey));
    if (pKeyStr)
    {
        pKeyStr->SetValStr(pszVal);
        return pKeyStr;
    }
	DeleteAtKey(pszKey);
	return SetStrNew(pszKey,pszVal);
}

CVarDefCont* CVarDefMap::SetStr( lpctstr pszName, bool fQuoted, lpctstr pszVal, bool fDeleteZero, bool fWarnOverwrite )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::SetStr");
	// ASSUME: This has been clipped of unwanted beginning and trailing spaces.
    ASSERT(pszName);
	if ( !pszName[0] )
		return nullptr;

    ASSERT(pszVal);
	if ( pszVal[0] == '\0' )	// if Val is an empty string, remove any previous def (and do not add a new def)
	{
		DeleteAtKey(pszName);
		return nullptr;
	}

	if ( !fQuoted && IsSimpleNumberString(pszVal))
	{
		// Just store the number and not the string.
		return SetNum( pszName, Exp_Get64Val( pszVal ), fDeleteZero);
	}

    size_t idx = m_Container.find_predicate(pszName, VarDefCompare);

	CVarDefCont * pVarBase = nullptr;
	if ( idx != SCONT_BADINDEX )
		pVarBase = m_Container[idx];

	if ( !pVarBase )
		return SetStrNew( pszName, pszVal );

	CVarDefContStr * pVarStr = dynamic_cast <CVarDefContStr *>( pVarBase );
	if ( pVarStr )
    {
        if ( fWarnOverwrite && !g_Serv.IsResyncing() && g_Serv.IsLoading() )
            DEBUG_WARN(( "Replacing existing VarStr '%s' with string: '%s'\n", pVarBase->GetKey(), pszVal ));
		pVarStr->SetValStr( pszVal );
    }
	else
	{
		if ( fWarnOverwrite && !g_Serv.IsResyncing() && g_Serv.IsLoading() )
			DEBUG_WARN(( "Replacing existing VarNum '%s' with string: '%s'\n", pVarBase->GetKey(), pszVal ));
		return SetStrOverride( pszName, pszVal );
	}
	return pVarStr;
}

CVarDefCont * CVarDefMap::GetKey( lpctstr pszKey ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetKey");
	CVarDefCont * pReturn = nullptr;

	if ( pszKey )
	{
        size_t idx = m_Container.find_predicate(pszKey, VarDefCompare);
		
		if ( idx != SCONT_BADINDEX )
			pReturn = m_Container[idx];
	}

	return pReturn;
}

int64 CVarDefMap::GetKeyNum( lpctstr pszKey ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetKeyNum");
	const CVarDefCont * pVar = GetKey(pszKey);
	if ( pVar == nullptr )
		return 0;
	return pVar->GetValNum();
}

lpctstr CVarDefMap::GetKeyStr( lpctstr pszKey, bool fZero ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetKeyStr");
	const CVarDefCont * pVar = GetKey(pszKey);
	if ( pVar == nullptr )
		return (fZero ? "0" : "");
	return pVar->GetValStr();
}

CVarDefCont * CVarDefMap::CheckParseKey( lpctstr pszArgs ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::CheckParseKey");
	tchar szTag[ EXPRESSION_MAX_KEY_LEN ];
	GetIdentifierString( szTag, pszArgs );
	CVarDefCont * pVar = GetKey(szTag);
	if ( pVar )
		return pVar;

	return nullptr;
}

CVarDefCont * CVarDefMap::GetParseKey_Advance( lpctstr & pszArgs ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetParseKey_Advance");
	// Skip to the end of the expression name.
	// The name can only be valid.

	tchar szTag[ EXPRESSION_MAX_KEY_LEN ];
    uint i = GetIdentifierString( szTag, pszArgs );
    pszArgs += i;

	CVarDefCont * pVar = GetKey(szTag);
	if ( pVar )
		return pVar;
	return nullptr;
}

bool CVarDefMap::GetParseVal_Advance( lpctstr & pszArgs, llong * pllVal ) const
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::GetParseVal_Advance");
	CVarDefCont * pVarBase = GetParseKey_Advance( pszArgs );
	if ( pVarBase == nullptr )
		return false;
	*pllVal = pVarBase->GetValNum();
	return true;
}

void CVarDefMap::DumpKeys( CTextConsole * pSrc, lpctstr pszPrefix ) const
{
	ADDTOCALLSTACK("CVarDefMap::DumpKeys");
	// List out all the keys.
	ASSERT(pSrc);
	if ( pszPrefix == nullptr )
		pszPrefix = "";

    bool fIsClient = pSrc->GetChar();
    for (const CVarDefCont* pVar : m_Container)
	{
        if (fIsClient)
        {
            pSrc->SysMessagef("%s%s=%s", pszPrefix, pVar->GetKey(), pVar->GetValStr());
        }
        else
        {
            g_Log.Event(LOGL_EVENT, "%s%s=%s\n", pszPrefix, pVar->GetKey(), pVar->GetValStr());
        }
	}
}

void CVarDefMap::ClearKeys(lpctstr mask)
{
	ADDTOCALLSTACK("CVarDefMap::ClearKeys");
	if ( mask && *mask )
	{
		if ( !m_Container.size() )
			return;

		CSString sMask(mask);
		sMask.MakeLower();

        size_t i = 0;
		iterator it = m_Container.begin();
		while ( it != m_Container.end() )
		{
            CVarDefCont *pVarBase = (*it);

			if ( pVarBase && ( strstr(pVarBase->GetKey(), sMask.GetPtr()) ) )
			{
				DeleteAt(i);
				it = m_Container.begin();
                i = 0;
			}
			else
			{
				++it;
                ++i;
			}
		}
	}
	else
	{
		Empty();
	}
}

/*
bool CVarDefMap::r_LoadVal( CScript & s )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::r_LoadVal");
	bool fQuoted = false;
    lpctstr ptcVal = s.GetArgStr( &fQuoted );
	return ( SetStr( s.GetKey(), fQuoted, ptcVal) ? true : false );
}
*/

void CVarDefMap::r_WritePrefix( CScript & s, lpctstr pszPrefix, lpctstr pszKeyExclude )
{
	ADDTOCALLSTACK_INTENSIVE("CVarDefMap::r_WritePrefix");
    if (m_Container.empty())
        return;

	TemporaryString tsZ;
	tchar* z = static_cast<tchar *>(tsZ);
	lpctstr	pszVal;
	bool fHasPrefix = (pszPrefix && *pszPrefix);
	bool fHasExclude = (pszKeyExclude && *pszKeyExclude);

    auto _WritePrefix = [&z, fHasPrefix, pszPrefix](lpctstr ptcKey) -> void
    {
        if ( fHasPrefix )
            sprintf(z, "%s.%s", pszPrefix, ptcKey);
        else
            sprintf(z, "%s", ptcKey);
    };

	// Write with any prefix.
    for (const CVarDefCont* pVar : m_Container)
	{
		if ( !pVar )
			continue;	// This should not happen, a warning maybe?

        lpctstr ptcKey = pVar->GetKey();
		if ( fHasExclude && !strcmpi(pszKeyExclude, ptcKey))
			continue;
		
        const CVarDefContNum * pVarNum = dynamic_cast<const CVarDefContNum*>(pVar);
        if (pVarNum)
        {
            // Save VarNums only if they are != 0, otherwise it's a waste of space in the save file
            if (pVarNum->GetValNum() != 0)
            {
                _WritePrefix(ptcKey);
                pszVal = pVar->GetValStr();
                s.WriteKey(z, pszVal);
            }
        }
        else
        {
            _WritePrefix(ptcKey);
            pszVal = pVar->GetValStr();
            s.WriteKeyFormat(z, "\"%s\"", pszVal);
        }
	}
}
