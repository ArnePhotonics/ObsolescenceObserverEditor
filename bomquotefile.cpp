#include "bomquotefile.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

SupplierName string_to_SupplierName(QString name) {
    if (name.toLower() == "farnell") {
        return SupplierName::farnell;
    } else if (name.toLower() == "digikey") {
        return SupplierName::digikey;
    } else if (name.toLower() == "mouser") {
        return SupplierName::mouser;
    } else {
        throw std::runtime_error(QString("supplier name \"%1\" is not recognized").arg(name).toStdString());
    }
};

QString SupplierName_to_string(SupplierName name) {
    switch (name) {
        case SupplierName::farnell:
            return "Farnell";
        case SupplierName::digikey:
            return "Digikey";
        case SupplierName::mouser:
            return "Mouser";
        default:
            throw std::runtime_error("unkown supplier enum value");
    }
};

BomQuoteFile::BomQuoteFile() {
    id_counter = 0;
    project_name = "test";
}

void BomQuoteFile::open_from_csv_file(QString file_name) {
    QFile bomquote_inputfile(file_name);
    bomquote_inputfile.open(QIODevice::ReadOnly);
    QTextStream bomquote_stream(&bomquote_inputfile);
    parts.clear();
    BomQuoteOrigPart orig_part;
    while (!bomquote_stream.atEnd()) {
        QString s = bomquote_stream.readLine();
        // Will be:
        // "0"|"orig"|"2"|"C1, C6"|"C0402C102J5GACTU"|"Kemet"|"CAP CER 0402 1n 50V
        // NP0 5%"|"cap_0402_highdens"

        auto cols = s.split("|");
        if (cols[1].remove('\"') == "orig") {
            if (!orig_part.is_empty()) {
                parts.append(orig_part);
            }
            orig_part.clear();
            orig_part.to_be_observed = true;
            orig_part.qty = cols[2].remove('\"').toInt();
            orig_part.refs = cols[3].remove('\"').split(", ");
            orig_part.orig_mpn = cols[4].remove('\"');
            orig_part.orig_manufacturer = cols[5].remove('\"');
            orig_part.orig_description = cols[6].remove('\"');
            orig_part.orig_footprint = cols[7].remove('\"');
        } else {
            BomQuoteSupplierPart supplier_part;
            supplier_part.supplier_name = string_to_SupplierName(cols[1].remove('\"'));
            supplier_part.sku = cols[4].remove('\"');
            QString supplier_mpn = cols[5].remove('\"');
            QString supplier_manufacturer = cols[6].remove('\"');
            supplier_part.description = cols[7].remove('\"');
            supplier_part.url = cols[14].remove('\"');
            if (supplier_mpn == orig_part.orig_mpn) {
                orig_part.supplier_parts.append(supplier_part);
            } else {
                BomQuoteAltPart alternative_part;
                alternative_part.to_be_observed = true;
                alternative_part.mpn = supplier_mpn;
                alternative_part.manufacturer = supplier_manufacturer;
                alternative_part.description = supplier_part.description;
                alternative_part.supplier_parts.append(supplier_part);
                orig_part.alternative_parts.append(alternative_part);
            }
        }
        //"0" | "Farnell" | "0" | "0" | "1535558" | "C0402C102J5GACTU" | "KEMET" |
        //"KEMET - C0402C102J5GACTU - Keramikvielschichtkondensator, SMD, 1000pF, 50
        // V, 0402 [Metrisch 1005], ± 5%, C0G / NP0" | "10" | "[10.0, 100.0, 500.0,
        // 2500.0, 5000.0]" | "[0.111, 0.048, 0.0192, 0.0163, 0.0141]" | "542207" |
        //"nichtAusUSA" | "1" | "de.farnell.com/b'1535558'"

        //"0" | "Farnell" | "0" |"0" | "8819556" | "GRM1555C1H102JA01D" | "MURATA" |
        //"MURATA - GRM1555C1H102JA01D - Keramikvielschichtkondensator, SMD, 1000
        // pF, 50 V, 0402 [Metrisch 1005], ± 5%, C0G / NP0" | "10" | "[10.0, 100.0,
        // 500.0, 2500.0, 5000.0]" | "[0.11, 0.0498, 0.024, 0.02, 0.0183]" |
        // "125063" |"nichtAusUSA" | "1" | "de.farnell.com/b'8819556'"

        // "0" | "DigiKey" | "0" |"0" | "399-10034-1-ND" | "C0402C102J5GACTU" |
        // "KEMET" | "Cap Ceramic 0.001uF 50V C0G 5% SMD 0402 125°C Paper T/R" | "1"
        // | "[1.0, 10.0, 100.0, 500.0, 1000.0, 2500.0, 5000.0]" |
        // "[0.1107011070110701, 0.08118081180811806, 0.03607011070110701,
        // 0.025756457564575643, 0.020230627306273063, 0.018394833948339483,
        // 0.01691881918819188]" |"505590" | "nichtAusUSA" | "1"
        // | "http://www.digikey.de/product-search/de?keywords=399-10034-1-ND"

        //"0" |"Mouser" | "0" | "0" | "80-C0402C102J5G" | "C0402C102J5GACTU" |
        //"KEMET" | "Cap Ceramic 0.001uF 50V C0G 5% SMD 0402 125°C Paper T/R" | "1"
        //|"[1.0, 10.0, 100.0, 500.0, 1000.0, 2000.0]" |       "[0.1107011070110701,
        // 0.08118081180811806, 0.035977859778597784, 0.024907749077490774,
        // 0.01937269372693727, 0.016605166051660514]" | "373263" | "nichtAusUSA" |
        //"1" | "https://www.mouser.de/mvc/header/search?keyword=80-C0402C102J5G"
    }
    if (!orig_part.is_empty()) {
        parts.append(orig_part);
    }
    bomquote_inputfile.close();
    set_filename(file_name);
    project_name = QFileInfo(file_name).baseName();
}

void BomQuoteFile::open_from_json_file(QString file_name) {
    QFile loadFile(file_name);
    loadFile.open(QIODevice::ReadOnly);
    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    QJsonObject obj = loadDoc.object();
    project_name = obj.keys().first();
    auto parts_json = obj[project_name].toArray();
    for (const auto &part_json : parts_json) {
        auto part_obj = part_json.toObject();
        BomQuoteOrigPart part;

        const auto &alternatives_array = part_obj["alts"].toArray();
        const auto &orig_obj = part_obj["orig"].toObject();
        const auto &refs = part_obj["refs"].toArray();
        for (const auto &ref : refs) {
            part.refs.append(ref.toString());
        }
        part.id_timestam_ms = part_obj["id_timestamp_ms"].toInt();
        part.qty = part_obj["qty"].toInt();
        part.to_be_observed = part_obj["to_be_observed"].toBool();
        part.orig_description = orig_obj["description"].toString();
        part.orig_erp_num = orig_obj["erp"].toString();
        part.orig_footprint = orig_obj["footprint"].toString();
        part.orig_manufacturer = orig_obj["manufacturer"].toString();
        part.orig_mpn = orig_obj["mpn"].toString();
        const auto supplier_array = orig_obj["suppliers"].toArray();
        for (const auto &supplier_val : supplier_array) {
            BomQuoteSupplierPart supplier;
            supplier.from_json(supplier_val.toObject());
            part.supplier_parts.append(supplier);
        }
        for (const auto &alternative_val : alternatives_array) {
            BomQuoteAltPart alternative;
            const auto &alternative_obj = alternative_val.toObject();
            alternative.description = alternative_obj["description"].toString();
            alternative.erp_num = alternative_obj["erp"].toString();
            alternative.id = alternative_obj["id"].toInt();
            alternative.manufacturer = alternative_obj["manufacturer"].toString();
            alternative.mpn = alternative_obj["mpn"].toString();
            alternative.to_be_observed = alternative_obj["to_be_observed"].toBool();
            const auto supplier_array = alternative_obj["suppliers"].toArray();
            for (const auto &supplier_val : supplier_array) {
                BomQuoteSupplierPart supplier;
                supplier.from_json(supplier_val.toObject());
                alternative.supplier_parts.append(supplier);
            }
            part.alternative_parts.append(alternative);
        }

        parts.append(part);
    }
    set_filename(file_name);
}

void BomQuoteFile::set_filename(QString fn) {
    auto fi = QFileInfo(fn);
    //if QFileInfo fi("/tmp/archive.tar.gz");
    // base = "archive"
    // QFileInfo::absolutePath(); Returns the file's path. This doesn't include the file name.
    QString base_name = fi.baseName();
    this->file_name = fi.absolutePath() + QDir::separator() + fi.baseName(); //extracts the file extension
}
void BomQuoteFile::save_to_json_file() {
    QJsonObject root_object;
    QJsonArray parts_root_array;
    for (auto &part : parts) {
        QJsonArray refs_array;
        for (const auto &s : part.refs) {
            refs_array.append(s);
        }

        QJsonArray supplier_array;
        for (const auto &s : part.supplier_parts) {
            supplier_array.append(s.to_json());
        }

        QJsonArray alternatives_array;
        for (auto &alt : part.alternative_parts) {
            QJsonObject alt_object;
            if (alt.id == 0) {
                alt.id = get_new_id_counter_value();
            }
            alt_object["id"] = alt.id;
            alt_object["mpn"] = alt.mpn;
            alt_object["manufacturer"] = alt.manufacturer;
            alt_object["to_be_observed"] = alt.to_be_observed;
            alt_object["description"] = alt.description;
            alt_object["erp"] = alt.erp_num;
            QJsonArray alt_supplier_array;
            for (const auto &s : alt.supplier_parts) {
                alt_supplier_array.append(s.to_json());
            }
            alt_object["suppliers"] = alt_supplier_array;
            alternatives_array.append(alt_object);
        }

        QJsonObject orig_obj;
        orig_obj["suppliers"] = supplier_array;
        orig_obj["mpn"] = part.orig_mpn;
        orig_obj["manufacturer"] = part.orig_manufacturer;
        orig_obj["description"] = part.orig_description;
        orig_obj["erp"] = part.orig_erp_num;
        orig_obj["footprint"] = part.orig_footprint;

        QJsonObject part_obj;
        part_obj["refs"] = refs_array;
        part_obj["orig"] = orig_obj;
        part_obj["alts"] = alternatives_array;

        part_obj["to_be_observed"] = part.to_be_observed;
        if (part.id_timestam_ms == 0) {
            part.id_timestam_ms = QDateTime::currentMSecsSinceEpoch();
            part.id_timestam_ms *= 10000;
            part.id_timestam_ms += get_new_id_counter_value();
        }
        part_obj["id_timestamp_ms"] = part.id_timestam_ms;
        part_obj["qty"] = part.qty;

        parts_root_array.append(part_obj);
    }
    root_object[project_name] = parts_root_array;

    QFile saveFile(file_name + ".json");
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        throw std::runtime_error(QObject::tr("Couldn't open save file.").toStdString());
    }
    QJsonDocument saveDoc(root_object);
    saveFile.write(saveDoc.toJson());
}

void BomQuoteFile::update_from_other_bomquote(const BomQuoteFile &other) {
    QList<int> new_parts;
    int other_part_index = 0;
    for (const auto &other_part : other.parts) {
        int found_match_at_me_index = -1;
        int me_index = 0;
        for (const auto &my_part : parts) {
            if (other_part.orig_mpn.toLower() == my_part.orig_mpn.toLower()) {
                found_match_at_me_index = me_index;
                break;
            }
            me_index++;
        }
        if (found_match_at_me_index == -1) { //not found here
            new_parts.append(other_part_index);
        } else {
            bool modified = false;
            auto &my_part = parts[found_match_at_me_index];
            if (my_part.qty != other_part.qty) {
                my_part.qty = other_part.qty;
                modified = true;
            }
            if (my_part.refs != other_part.refs) {
                my_part.refs = other_part.refs;
                modified = true;
            }
            my_part.modified = modified;
        }
        other_part_index++;
    }
    for (const auto &new_part_index : new_parts) {
        auto new_part = other.parts[new_part_index];
        new_part.modified = true;
        parts.append(new_part);
    }
}

int BomQuoteFile::get_new_id_counter_value() {
    id_counter++;
    qDebug() << id_counter;
    return id_counter;
}

bool BomQuoteOrigPart::is_empty() {
    return orig_mpn.isEmpty();
}

void BomQuoteOrigPart::clear() {
    modified = false;
    supplier_parts.clear();
    alternative_parts.clear();
    refs.clear();
    orig_mpn.clear();
    orig_manufacturer.clear();
    orig_erp_num.clear();
    orig_footprint.clear();
    orig_description.clear();
    to_be_observed = false;
    qty = 0;
    id_timestam_ms = 0;
}

QJsonObject BomQuoteSupplierPart::to_json() const {
    QJsonObject supplier_obj;
    supplier_obj["supplier_name"] = SupplierName_to_string(supplier_name);
    supplier_obj["sku"] = sku;
    supplier_obj["url"] = url;
    supplier_obj["description"] = description;
    return supplier_obj;
}

void BomQuoteSupplierPart::from_json(QJsonObject obj) {
    supplier_name = string_to_SupplierName(obj["supplier_name"].toString());
    sku = obj["sku"].toString();
    description = obj["description"].toString();
    url = obj["url"].toString();
}

BomQuoteAltPart::BomQuoteAltPart() {
    id = 0;
}
