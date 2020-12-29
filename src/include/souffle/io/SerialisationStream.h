/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file SerialisationStream.h
 *
 * Defines a common base class for relation serialisation streams.
 *
 ***********************************************************************/

#pragma once

#include "souffle/RamTypes.h"

#include "souffle/utility/StringUtil.h"
#include "souffle/utility/json11.h"
#include <cassert>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

class RecordTable;
class SymbolTable;

using json11::Json;

template <bool readOnlyTables>
class SerialisationStream {
public:
    virtual ~SerialisationStream() = default;

protected:
    template <typename A>
    using RO = std::conditional_t<readOnlyTables, const A, A>;

    SerialisationStream(RO<SymbolTable>& symTab, RO<RecordTable>& recTab, Json types,
            std::vector<std::string> relTypes, size_t auxArity = 0)
            : symbolTable(symTab), recordTable(recTab), types(std::move(types)),
              typeAttributes(std::move(relTypes)), arity(typeAttributes.size() - auxArity),
              auxiliaryArity(auxArity) {}

    SerialisationStream(RO<SymbolTable>& symTab, RO<RecordTable>& recTab, Json types)
            : symbolTable(symTab), recordTable(recTab), types(std::move(types)) {
        setupFromJson();
    }

    SerialisationStream(RO<SymbolTable>& symTab, RO<RecordTable>& recTab,
            const std::map<std::string, std::string>& rwOperation)
            : symbolTable(symTab), recordTable(recTab) {
        std::string parseErrors;
        types = Json::parse(rwOperation.at("types"), parseErrors);
        assert(parseErrors.size() == 0 && "Internal JSON parsing failed.");

        auxiliaryArity = RamSignedFromString(rwOperation.at("auxArity"));

        setupFromJson();
    }

    RO<SymbolTable>& symbolTable;
    RO<RecordTable>& recordTable;
    Json types;
    std::vector<std::string> typeAttributes;

    size_t arity = 0;
    size_t auxiliaryArity = 0;

private:
    void setupFromJson() {
        auto&& relInfo = types["relation"];
        arity = static_cast<size_t>(relInfo["arity"].long_value());

        assert(relInfo["types"].is_array());
        auto&& relTypes = relInfo["types"].array_items();
        assert(relTypes.size() == arity);

        for (size_t i = 0; i < arity; ++i) {
            auto&& type = relTypes[i].string_value();
            assert(!type.empty() && "malformed types tag");
            typeAttributes.push_back(type);
        }

        for (size_t i = 0; i < auxiliaryArity; i++) {
            typeAttributes.push_back("i:number");
        }
    }
};

}  // namespace souffle
