#include "mainwindow.h"

#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::load_bomquote_file_into_gui() {
    ui->treeWidget->clear();
    int part_index = 0;
    for (const auto &part : bomquote_file.parts) {
        auto part_item = new QTreeWidgetItem(QStringList{part.refs.join(", "), part.orig_mpn, part.orig_manufacturer, part.orig_description});
        if (part.to_be_observed) {
            part_item->setCheckState(0, Qt::Checked);
        } else {
            part_item->setCheckState(0, Qt::Unchecked);
        }
        part_item->setData(0, Qt::UserRole, part_index);
        QTreeWidgetItem *orig_supplier = nullptr;
        for (const auto &supplier_part : part.supplier_parts) {
            if (orig_supplier == nullptr) {
                orig_supplier = new QTreeWidgetItem(QStringList{"supplier"});
            }
            auto item_supplier =
                new QTreeWidgetItem(QStringList{SupplierName_to_string(supplier_part.supplier_name), supplier_part.sku, "", supplier_part.description});
            item_supplier->setData(2, Qt::UserRole, supplier_part.url);
            orig_supplier->addChild(item_supplier);
        }
        int alt_index = 0;
        QTreeWidgetItem *alt_parts = nullptr;
        for (const auto &alternative_part : part.alternative_parts) {
            if (alt_parts == nullptr) {
                alt_parts = new QTreeWidgetItem(QStringList{"Alternatives"});
                alt_parts->setCheckState(0, Qt::Checked);
            }
            auto item_alternative =
                new QTreeWidgetItem(QStringList{"Alternative", alternative_part.mpn, alternative_part.manufacturer, alternative_part.description});
            if (alternative_part.to_be_observed) {
                item_alternative->setCheckState(0, Qt::Checked);
            } else {
                item_alternative->setCheckState(0, Qt::Unchecked);
            }
            item_alternative->setData(0, Qt::UserRole, part_index);
            item_alternative->setData(1, Qt::UserRole, alt_index);
            for (const auto &supplier_part : alternative_part.supplier_parts) {
                auto item_supplier =
                    new QTreeWidgetItem(QStringList{SupplierName_to_string(supplier_part.supplier_name), supplier_part.sku, "", supplier_part.description});
                item_supplier->setData(2, Qt::UserRole, supplier_part.url);
                item_alternative->addChild(item_supplier);
            }
            alt_parts->addChild(item_alternative);
            alt_index++;
        }
        if (orig_supplier) {
            part_item->addChild(orig_supplier);
        }
        if (alt_parts) {
            part_item->addChild(alt_parts);
        }

        ui->treeWidget->addTopLevelItem(part_item);
        part_index++;
    }
    ui->treeWidget->expandAll();
}

void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column) {
    if (item->checkState(0) == Qt::Unchecked) {
        item->setExpanded(false);
        item->parent();
        for (int i = 0; i < item->childCount(); i++) {
            auto child = item->child(i);
            if (child->checkState(0) == Qt::Checked) {
                child->setCheckState(0, Qt::Unchecked);
            }
        }
    }
    bool part_index_ok = false;
    int part_index = item->data(0, Qt::UserRole).toInt(&part_index_ok);
    bool alt_index_ok = false;
    int alt_index = item->data(0, Qt::UserRole).toInt(&alt_index_ok);

    if (item->checkState(0) == Qt::Checked) {
        if (part_index_ok) {
            auto &part = bomquote_file.parts[part_index];
            if (alt_index_ok) {
                auto &alt = part.alternative_parts[alt_index];
                alt.to_be_observed = true;
            } else {
                part.to_be_observed = true;
            }
        }
    } else if (item->checkState(0) == Qt::Unchecked) {
        if (part_index_ok) {
            auto &part = bomquote_file.parts[part_index];
            if (alt_index_ok) {
                auto &alt = part.alternative_parts[alt_index];
                alt.to_be_observed = false;
            } else {
                part.to_be_observed = false;
            }
        }
    }
}

void MainWindow::on_actionopen_triggered() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("bomQuotes (*.bomQuote)"));

    if (dialog.exec()) {
        QString file_name;
        file_name = dialog.selectedFiles()[0];
        bomquote_file.open_from_csv_file(file_name);
        load_bomquote_file_into_gui();
    }
}

void MainWindow::on_actionsave_observer_file_triggered() {
    bomquote_file.save_to_json_file();
}
