///
/// \package astlib
/// \file CodecDeclarationLoader.cpp
///
/// \author Marian Krivos <marian.krivos@rsys.sk>
/// \date 31Jan.,2017 
/// \brief definicia typu
///
/// (C) Copyright 2017 R-SYS s.r.o
/// All rights reserved.
///

#include "CodecDeclarationLoader.h"
#include "model/CodecDescription.h"
#include "model/CategoryDescription.h"
#include "model/VariableItemDescription.h"
#include "model/RepetitiveItemDescription.h"
#include "Exception.h"

#include "Poco/XML/XMLStreamParser.h"
#include <Poco/XML/Content.h>
#include "Poco/SAX/InputSource.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/AutoPtr.h"
#include <Poco/AutoPtr.h>
#include "Poco/Exception.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"

#include <fstream>

using namespace Poco::XML;

namespace astlib
{

CodecDeclarationLoader::CodecDeclarationLoader()
{
}

CodecDeclarationLoader::~CodecDeclarationLoader()
{
}

CodecDescriptionPtr CodecDeclarationLoader::load(const std::string& filename)
{
    CodecDescriptionPtr codecDescription(new CodecDescription);
    Poco::XML::InputSource src(filename);
    Poco::XML::NamePool* pool = new Poco::XML::NamePool(3571);
    Poco::XML::DOMParser parser(pool);
    pool->release();

    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, true);
    parser.setFeature(Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true);

    Poco::XML::AutoPtr<Poco::XML::Document> document = parser.parse(&src);
    poco_check_ptr (document);

    auto root = Poco::XML::AutoPtr<Element>(document->documentElement(), true);

    if (root->nodeName() != "Category")
    {
        throw Poco::DataFormatException("in CodecDeclarationLoader::load() - no 'Category' element at top level");
    }

    loadCategory(*codecDescription, *root);

    return codecDescription;
}

void CodecDeclarationLoader::loadCategory(CodecDescription& codecDescription, const Element& root)
{
    CategoryDescription categoryDescription;

    int cat = Poco::NumberParser::parse(root.getAttribute("id"));
    categoryDescription.setCategory(cat);
    categoryDescription.setEdition(root.getAttribute("ver"));
    categoryDescription.setFamily("ast");
    categoryDescription.setDescription(root.getAttribute("name"));
    codecDescription.addCategoryDescription(categoryDescription);

    for (auto node = root.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element)
        {
            auto name = element->nodeName();
            std::cout << " " << name << std::endl;
            if (name == "DataItem")
            {
                ItemDescriptionPtr item = loadDataItem(*element);
                if (item)
                    codecDescription.addDataItem(item);
            }
        }
    }

    for (auto node = root.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element)
        {
            auto name = element->nodeName();
            if (name == "UAP")
            {
                loadUap(codecDescription, *element);
            }
        }
    }
}

void CodecDeclarationLoader::loadUap(CodecDescription& codecDescription, const Element& parent)
{
    for (auto node = parent.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element)
        {
            auto name = element->nodeName();
            if (name == "UAPItem")
            {
                auto bit = element->getAttribute("bit");
                auto idString = element->innerText();
                int id = 0;
                if (idString == "SP")
                    id = ItemDescription::SP;
                else if (idString == "RE")
                    id = ItemDescription::RE;
                else if (idString == "-")
                    id = ItemDescription::FX;
                else
                    id = Poco::NumberParser::parse(idString);
                std::cout << " " << name << " " << bit << " " << id << std::endl;
            }
        }
    }
}

ItemDescriptionPtr CodecDeclarationLoader::loadDataItem(const Element& element)
{
    auto idString = element.getAttribute("id");
    int id = 0;
    if (idString == "SP")
        id = ItemDescription::SP;
    else if (idString == "RE")
        id = ItemDescription::RE;
    else
        id = Poco::NumberParser::parse(idString);

    auto description = element.getChildElement("DataItemName")->innerText();
    Element* formatElement = dynamic_cast<Element*>(element.getChildElement("DataItemFormat")->firstChild());
    poco_assert(formatElement);

    ItemFormat format = ItemFormat(formatElement->nodeName());

    std::cout << "    " << id << " " << format.toString() << " " << description << std::endl;

    switch(format.toValue())
    {
        case ItemFormat::Fixed:
            return loadFixedDeclaration(id, description, *formatElement);

        case ItemFormat::Variable:
            return loadVariableDeclaration(id, description, *formatElement);

        case ItemFormat::Repetitive:
            return loadRepetitiveDeclaration(id, description, *formatElement);

        case ItemFormat::Compound:
            return loadCompoundDeclaration(id, description, *formatElement);

        case ItemFormat::Explicit:
            return loadExplicitDeclaration(id, description, *formatElement);

        default:
            throw Exception("CodecDeclarationLoader::loadDataItem(): unknown item type " + format.toString());
    }

    return nullptr;
}

ItemDescriptionPtr CodecDeclarationLoader::loadFixedDeclaration(int id, const std::string& description, const Element& element)
{
    Fixed fixed = loadFixed(element);
    return std::make_shared<FixedItemDescription>(id, description, fixed);
}

Fixed CodecDeclarationLoader::loadFixed(const Element& element)
{
    int length = Poco::NumberParser::parse(element.getAttribute("length"));
    BitsDescriptionArray bitsArray = loadBitsDeclaration(element);
    return Fixed(bitsArray, length);
}

ItemDescriptionPtr CodecDeclarationLoader::loadVariableDeclaration(int id, const std::string& description, const Element& parent)
{
    FixedVector fixeds;
    for (auto node = parent.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element && element->nodeName() == "Fixed")
        {
            Fixed fixed = loadFixed(*element);
            fixeds.push_back(fixed);
        }
    }
    return std::make_shared<VariableItemDescription>(id, description, fixeds);
}

ItemDescriptionPtr CodecDeclarationLoader::loadRepetitiveDeclaration(int id, const std::string& description, const Element& parent)
{
    FixedVector fixeds;
    for (auto node = parent.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element && element->nodeName() == "Fixed")
        {
            Fixed fixed = loadFixed(*element);
            fixeds.push_back(fixed);
        }
    }
    return std::make_shared<RepetitiveItemDescription>(id, description, fixeds);
}

ItemDescriptionPtr CodecDeclarationLoader::loadCompoundDeclaration(int id, const std::string& description, const Element& element)
{
    return nullptr;
}

ItemDescriptionPtr CodecDeclarationLoader::loadExplicitDeclaration(int id, const std::string& description, const Element& element)
{
    return nullptr;
}

BitsDescriptionArray CodecDeclarationLoader::loadBitsDeclaration(const Element& parent)
{
    BitsDescriptionArray bitsArray;

    for (auto node = parent.firstChild(); node; node = node->nextSibling())
    {
        const Element* element = dynamic_cast<Element*>(node);
        if (element && element->nodeType() == Node::ELEMENT_NODE && element->nodeName() == "Bits")
        {
            BitsDescription bits;

            if (element->hasAttribute("bit"))
            {
                bits.bit = Poco::NumberParser::parse(element->getAttribute("bit"));
                if (element->hasAttribute("fx"))
                {
                    bits.fx = Poco::NumberParser::parse(element->getAttribute("fx"));
                }
            }
            else
            {
                bits.from = Poco::NumberParser::parse(element->getAttribute("from"));
                bits.to = Poco::NumberParser::parse(element->getAttribute("to"));

                // Enumerations
                const Element* node = dynamic_cast<const Element*>(element->getChildElement("BitsValue"));
                for(; node; node = dynamic_cast<const Element*>(node->nextSibling()))
                {
                    if (node->nodeName() == "BitsValue")
                    {
                        int value = Poco::NumberParser::parse(node->getAttribute("val"));
                        std::string key = node->innerText();
                        bits.addEnumeration(key, value);
                        std::cout << "     enum " << key << " = " << value << std::endl;
                    }
                }
            }

            bits.name = dynamic_cast<const Element*>(element->getChildElement("BitsShortName"))->innerText();
            std::cout << "      " << bits.toString() << std::endl;
            bitsArray.push_back(bits);
        }
    }

    return bitsArray;
}

} /* namespace codecDescription */
