#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/File.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

class JsonDoc
{
public:
    JsonDoc();
    ~JsonDoc();

    bool ParseFile(const Path& filePath);
    rapidjson::Value& GetRoot() { return m_doc.GetObject(); }

private:
    rapidjson::Document m_doc;

}; // class JsonDoc
