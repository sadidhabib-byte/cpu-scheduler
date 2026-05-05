#pragma once

#include <QMainWindow>

#include "scheduler.hpp"

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTableWidget;

class GanttWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void on_process_count_changed(int n);
    void on_algorithm_changed(int index);
    void on_random_clicked();
    void on_run_clicked();
    void on_theme_toggle_clicked();

private:
    void apply_theme(bool dark_mode);
    void setup_tables();
    void fill_comparison_view(const std::vector<Process>& original);
    [[nodiscard]] std::vector<Process> collect_processes_from_table();

    QSpinBox* spin_count_;
    QComboBox* combo_algo_;
    QSpinBox* spin_quantum_;
    QLabel* label_quantum_;
    QTableWidget* table_input_;
    QPushButton* btn_random_;
    QPushButton* btn_run_;
    QPushButton* btn_theme_;

    QStackedWidget* stack_results_;
    QWidget* page_single_;
    QTableWidget* table_results_;
    QLabel* label_avg_;
    GanttWidget* gantt_;

    QWidget* page_compare_;
    QTableWidget* table_compare_;
    bool dark_mode_enabled_ = false;
};
