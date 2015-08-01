/*
Part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#include "xml_import_export.h"
#include "..\version.h"

#include "..\debug\debuglog.h"

// A global Windows "basic string". Actual memory is allocated by the
// COM methods used by MSXML which take &keyconf_bstr. We must use SysFreeString() 
// to free this memory before subsequent uses, to prevent a leak.
BSTR xmlParser_bstr;

bool generate_xml_for_propset(SavedPropSet* props, std::string outputFile)
{
	//Create the XML
	IXMLDOMDocumentPtr pXMLDoc;
	HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (FAILED(hr))
	{
		write_text_to_log_file("Failed to create the XML class instance");
		return false;
	}

	VARIANT_BOOL bIsSuccessful;
	if (FAILED(pXMLDoc->loadXML(L"<object-set></object-set>", &bIsSuccessful)))
	{
		write_text_to_log_file("Root creation failed");
		handle_error(pXMLDoc);
		return false;
	}

	IXMLDOMProcessingInstructionPtr pXMLProcessingNode;
	pXMLDoc->createProcessingInstruction(L"xml", L" version=\"1.0\" encoding=\"UTF-8\"", &pXMLProcessingNode);

	//Get the root element just created    
	IXMLDOMElementPtr pXMLRootElem;
	pXMLDoc->get_documentElement(&pXMLRootElem);

	_variant_t vtObject;
	vtObject.vt = VT_DISPATCH;
	vtObject.pdispVal = pXMLRootElem;
	vtObject.pdispVal->AddRef();
	pXMLDoc->insertBefore(pXMLProcessingNode, vtObject, 0);

	//Add an attribute
	pXMLRootElem->setAttribute(L"set-name", _variant_t(props->saveName.c_str()));
	pXMLRootElem->setAttribute(L"ent-version", _variant_t(VERSION_STRING.c_str()));

	for each (SavedPropDBRow* row in props->items)
	{
		//Create child element
		IXMLDOMElementPtr objectNode;
		pXMLDoc->createElement(L"object", &objectNode);

		objectNode->setAttribute(L"title", _variant_t(row->title.c_str()));
		objectNode->setAttribute(L"posX", _variant_t(row->posX));
		objectNode->setAttribute(L"posY", _variant_t(row->posY));
		objectNode->setAttribute(L"posZ", _variant_t(row->posZ));

		pXMLRootElem->appendChild(objectNode, 0);
	}

	char Path[MAX_PATH];

	

	if (FAILED(pXMLDoc->save(_variant_t(outputFile.c_str()))))
	{
		write_text_to_log_file("Save failed");
		write_text_to_log_file(outputFile);
		handle_error(pXMLDoc);
		return false;
	}
	else
	{
		write_text_to_log_file("Save complete");
		write_text_to_log_file(outputFile);
		return true;
	}
}

SavedPropSet* parse_xml_for_propset(HANDLE inputFile)
{
	CoInitialize(NULL);

	//read XML
	MSXML2::IXMLDOMDocumentPtr spXMLDoc;
	spXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (!spXMLDoc->load(inputFile))
	{
		write_text_to_log_file("No config found, using defaults");
	}
	return false;
}

void handle_error(IXMLDOMDocumentPtr doc)
{
	std::ostringstream ss;
	IXMLDOMParseError* pError;
	ss << "XML error: ";
	doc->get_parseError(&pError);
	if (pError)
	{
		BSTR reasonStr;
		pError->get_reason(&reasonStr);
		ss << reasonStr;
	}
	else
	{
		ss << "Unknown";
	}
	write_text_to_log_file(ss.str());
}