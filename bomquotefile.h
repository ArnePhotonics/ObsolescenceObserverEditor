#ifndef BOMQUOTEFILE_H
#define BOMQUOTEFILE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <stdexcept>

enum class SupplierName { digikey, farnell, mouser };

SupplierName string_to_SupplierName(QString name);
QString SupplierName_to_string(SupplierName name);

class BomQuoteSupplierPart {
    public:
    SupplierName supplier_name;
    QString sku;
    QString description;
    QString url;
    QJsonObject to_json() const;
};

class BomQuoteAltPart {
    public:
    bool to_be_observed;
    QString mpn;
    QString manufacturer;
    QString erp_num;
    QString description;
    QList<BomQuoteSupplierPart> supplier_parts;
};

class BomQuoteOrigPart {
    public:
    bool to_be_observed;
    QString orig_mpn;
    QString orig_manufacturer;
    QString orig_erp_num;
    QString orig_footprint;
    QString orig_description;
    int qty;
    QStringList refs;
    int id_timestam_ms;
    QList<BomQuoteSupplierPart> supplier_parts;
    QList<BomQuoteAltPart> alternative_parts;
    bool is_empty();
    void clear();
};

class BomQuoteFile {
    public:
    BomQuoteFile();
    void open_from_csv_file(QString file_name);
    void save_to_json_file();
    QList<BomQuoteOrigPart> parts;
    QString project_name;
    QString file_name;
};

#endif // BOMQUOTEFILE_H
