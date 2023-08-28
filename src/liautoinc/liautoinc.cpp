#include "liautoinc.h"
#include "greptime/v1/column.pb.h"
#include "src/core.h"

namespace liatoinc {

std::unordered_map<int, std::vector<std::tuple<std::string, ColumnDataType>>> signalNameAndSchemaMap;

// std::unordered_map<int, std::vector<std::string>> signalNameMap;
// std::unordered_map<int, std::vector<ColumnDataType>> SignalSchemaMap;

void setCanIdSignalNameList(std::unordered_map<
                int, 
                std::vector<std::tuple<std::string, ColumnDataType>>> &signalNameAndSchemaMap) {

    liatoinc::signalNameAndSchemaMap = signalNameAndSchemaMap;
}

using Variant = std::variant<
                bool,
                int8_t,
                int16_t,
                int32_t,
                int64_t,
                uint8_t,
                uint16_t,
                uint32_t,
                uint64_t,
                float,
                double,
                std::string
                >;

void addValue(Column_Values *values, ColumnDataType datatype, Variant varValue) {
    switch (datatype) {
        case ColumnDataType::BOOLEAN: {
            if (!std::holds_alternative<bool>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<bool>(varValue);
            values->add_bool_values(value);
            std::cout << "Bool: " << value << std::endl;
            break;
        }
        case ColumnDataType::INT8: {
            if (!std::holds_alternative<int8_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<int8_t>(varValue);
            values->add_i8_values(value);
            std::cout << "Int8: " << value << std::endl;
            break;
        }
        case ColumnDataType::INT16: {
            if (!std::holds_alternative<int16_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<int16_t>(varValue);
            values->add_i16_values(value);
            std::cout << "Int16: " << value << std::endl;
            break;
        }
        case ColumnDataType::INT32: {
            if (!std::holds_alternative<int32_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<int32_t>(varValue);
            values->add_i32_values(value);
            std::cout << "Int32: " << value << std::endl;
            break;
        }
        case ColumnDataType::INT64: {
            if (!std::holds_alternative<int64_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<int64_t>(varValue);
            values->add_i64_values(value);
            std::cout << "Int64: " << value << std::endl;
            break;
        }
        case ColumnDataType::UINT8: {
            if (!std::holds_alternative<uint8_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<uint8_t>(varValue);
            values->add_u8_values(value);
            std::cout << "Uint8: " << value << std::endl;
            break;
        }
        case ColumnDataType::UINT16: {
            if (!std::holds_alternative<uint16_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<uint16_t>(varValue);
            values->add_u16_values(value);
            std::cout << "Uint16: " << value << std::endl;
            break;
        }
        case ColumnDataType::UINT32: {
            if (!std::holds_alternative<uint32_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<uint32_t>(varValue);
            values->add_u32_values(value);
            std::cout << "Uint32: " << value << std::endl;
            break;
        }
        case ColumnDataType::UINT64: {
            if (!std::holds_alternative<uint64_t>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<uint64_t>(varValue);
            values->add_u64_values(value);
            std::cout << "Uint64: " << value << std::endl;
            break;
        }
        case ColumnDataType::FLOAT32: {
            if (!std::holds_alternative<float>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<float>(varValue);
            values->add_f32_values(value);
            std::cout << "Float32: " << value << std::endl;
            break;
        }
        case ColumnDataType::FLOAT64: {
            if (!std::holds_alternative<double>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<double>(varValue);
            values->add_f64_values(value);
            std::cout << "Float64: " << value << std::endl;
            break;
        }
        case ColumnDataType::STRING: {
            if (!std::holds_alternative<std::string>(varValue)) {
                throw std::logic_error("The data type indicated by the schema is inconsistent with the real data type");
            }
            auto value = std::get<std::string>(varValue);
            values->add_string_values(value);
            std::cout << "String: " << value << std::endl;
            break;
        }
        default:
            break;
    }

}

/*
 * canIdSizeMap 　canid -> canidSize(表示该canid有多少条数据)
 ×　timeStampVec　canid->时间戳列表　(表示canid各条数据的时间戳)
 *　valuesMap　　canid->{canid各条数据集合，每条数据对应某个时间戳canid的value集合}
 */
void commitData(std::map<int, int>  &canIdSizeMap,
                        std::map<int,std::shared_ptr<std::vector<long>>> &timeStampVec,
                        std::map<int,std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<Variant>>>>>>> &valuesMap) {
    
    std::vector<InsertRequest> insReqVec;
    insReqVec.reserve(canIdSizeMap.size());

    for (const auto &[canid, n] : canIdSizeMap) {

        const auto & tsVec = timeStampVec[canid];
        const auto & valuesVec = valuesMap[canid];
        const auto & nameAndSchema = signalNameAndSchemaMap[canid];

        if(tsVec->size() != valuesVec->size()) {
            throw std::logic_error("The timestamp is inconsistent with the number of data rows");
        }

        if (n == 0) {
            return;
        }
        int m = valuesVec->at(0)->size();

        std::string table_name = "table_" + std::to_string(canid);

        InsertRequest insReq;
        insReq.set_table_name(table_name);
        insReq.set_row_count(n);
        // timestamp
        {
            Column column;
            column.set_column_name("ts");
            column.set_semantic_type(Column_SemanticType::Column_SemanticType_TIMESTAMP);
            column.set_datatype(ColumnDataType::TIMESTAMP_MILLISECOND);
            auto values = column.mutable_values();
            for (const auto& ts : *tsVec) {
                values->add_ts_millisecond_values(ts);
            }
            insReq.add_columns()->Swap(&column);
        }

        // n rows, m columns
        // std::cout << "n: " << n << '\n';
        // std::cout << "m: " << m << '\n';
        for (int j = 0; j < m; ++j) {
            const auto &[column_name, datatype] = nameAndSchema[j];
            Column column;
            column.set_column_name(column_name);
            column.set_semantic_type(Column_SemanticType::Column_SemanticType_FIELD);
            column.set_datatype(datatype);
            auto values = column.mutable_values();
            for (int i = 0; i < n; ++i) {
                assert(m == valuesVec->at(i)->size());
                const std::vector<Variant> &fields = *valuesVec->at(i)->at(j);
                if (fields.size() == 0) {
                    // null
                    throw std::logic_error("field is empty");
                } else if(fields.size() == 1) {
                    const Variant &value = fields[0];
                    addValue(values, datatype, value);

                } else {
                    // array [1, 2] -> std::string
                    nlohmann::json jsonArray = nlohmann::json::array();
                    for (const auto &v : fields) {
                        std::visit([&jsonArray](auto&& arg) {
                            jsonArray.push_back(arg);
                        }, v);
                    }
                    std::string value = jsonArray.dump();
                    // std::cout << "json array: " << value << std::endl;
                    values->add_string_values(value);
                }
            }
            insReq.add_columns()->Swap(&column);
        }
        insReqVec.push_back(std::move(insReq));
    }

    greptime::Database database("public", "localhost:4001");
    auto stream_inserter = database.CreateStreamInserter();

    stream_inserter.Write(std::move(insReqVec));
    stream_inserter.WriteDone();

    grpc::Status status = stream_inserter.Finish();
    std::cout << "Finish" << std::endl;

    if (status.ok()) {
        std::cout << "success!" << std::endl;
        auto response = stream_inserter.GetResponse();

        std::cout << "notice: [";
        std::cout << response.affected_rows().value() << "] ";
        std::cout << "rows of data are successfully inserted into the public database"<< std::endl;
    } else {
        std::cout << "fail!" << std::endl;
        std::string emsg = "error message: " + status.error_message() + "\nerror details: " + status.error_details() + "\n"; 
        throw std::runtime_error(emsg);
    }
}
}


int main() {

    std::string vname1 = "IntN";
    liatoinc::ColumnDataType datatype1 = liatoinc::ColumnDataType::INT32;
    std::vector<liatoinc::Variant> value1 = {(int32_t)1};

    liatoinc::ColumnDataType datatype2 = liatoinc::ColumnDataType::FLOAT64;
    std::string vname2 = "DoubleN";
    std::vector<liatoinc::Variant> value2 = {(double)1.1};

    liatoinc::ColumnDataType datatype3 = liatoinc::ColumnDataType::STRING;
    std::string vname3 = "ArrayN";
    std::vector<liatoinc::Variant> value3 = {(double)1.1, (double)2.2, (double)3.3, (double)4.4};
    
    int canid = 114514;

    // column and schema
    liatoinc::signalNameAndSchemaMap.emplace(
        canid,
        std::vector<std::tuple<std::string, liatoinc::ColumnDataType>>{
            std::make_tuple(vname1, datatype1),
            std::make_tuple(vname2, datatype2),
            std::make_tuple(vname3, datatype3)
        }
    );

    // values
    std::map<int, int> canIdSizeMap;
    canIdSizeMap.emplace(canid, 1);

    std::map<int,std::shared_ptr<std::vector<long>>> timeStampVec;
    timeStampVec.emplace(
        canid,
        std::make_shared<std::vector<long>>(
            std::vector<long>{
                111111111,
                111111112,
            }
        )
    );

    std::map<int,std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>>>>>> valuesMap;

    std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>> row1 {
        std::make_shared<std::vector<liatoinc::Variant>>(value1),
        std::make_shared<std::vector<liatoinc::Variant>>(value2),
        std::make_shared<std::vector<liatoinc::Variant>>(value3)
    };
    std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>>>> row {
        std::make_shared<std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>>>(row1),
        std::make_shared<std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>>>(row1),
    };
    
    valuesMap.emplace(canid, std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<liatoinc::Variant>>>>>>(row));
    try {
        liatoinc::commitData(canIdSizeMap, timeStampVec, valuesMap);
    }
    catch (const std::exception& e) // caught by reference to base
    {
        std::cout << " a standard exception was caught, with message '"
                  << e.what() << "'\n";
    }
    return 0;
}
