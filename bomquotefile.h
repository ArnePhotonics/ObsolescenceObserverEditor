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
    void from_json(QJsonObject obj);
};

class BomQuoteAltPart {
    public:
    BomQuoteAltPart();
    bool to_be_observed;
    QString mpn;
    QString manufacturer;
    QString erp_num;
    QString description;
    QList<BomQuoteSupplierPart> supplier_parts;
    int id;
};

class BomQuoteOrigPart {
    public:
    BomQuoteOrigPart() {
        to_be_observed = true;
        modified = false;
        qty = 0;
        id_timestam_ms = 0;
    }
    bool to_be_observed;
    bool modified;
    QString orig_mpn;
    QString orig_manufacturer;
    QString orig_erp_num;
    QString orig_footprint;
    QString orig_description;
    int qty;
    QStringList refs;
    int64_t id_timestam_ms;
    QList<BomQuoteSupplierPart> supplier_parts;
    QList<BomQuoteAltPart> alternative_parts;
    bool is_empty();
    void clear();
};

class BomQuoteFile {
    public:
    BomQuoteFile();
    void open_from_csv_file(QString file_name);
    void open_from_json_file(QString file_name);
    void save_to_json_file();
    void update_from_other_bomquote(const BomQuoteFile &other);
    QList<BomQuoteOrigPart> parts;
    QString project_name;
    QString file_name;

    private:
    void set_filename(QString fn);
    int get_new_id_counter_value();
    int id_counter;
};

#endif // BOMQUOTEFILE_H
