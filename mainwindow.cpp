#include "mainwindow.h"

#include <QComboBox>
#include <QDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <optional>

namespace {

constexpr int k_algo_compare_index = 5;

QColor color_for_pid(int pid, int max_pid) {
    if (pid <= 0 || max_pid <= 0) return QColor(180, 180, 180);
    const float hue = 360.0f * (static_cast<float>(pid - 1) / static_cast<float>(max_pid));
    QColor c;
    c.setHsvF(std::fmod(hue / 360.0f, 1.0f), 0.55f, 0.92f);
    return c;
}

QString light_theme_style_sheet() {
    return QStringLiteral(
        "QMainWindow { background-color: #f5f7fb; }"
        "QLabel { color: #111827; font-size: 13px; }"
        "QPushButton { background-color: #2563eb; color: #ffffff; border: 0; border-radius: 8px;"
        "              padding: 7px 12px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1d4ed8; }"
        "QPushButton:pressed { background-color: #1e40af; }"
        "QPushButton#themeButton { background-color: #334155; }"
        "QPushButton#themeButton:hover { background-color: #1f2937; }"
        "QComboBox, QSpinBox, QLineEdit { color: #111827; background: #ffffff;"
        "                               border: 1px solid #94a3b8; border-radius: 6px;"
        "                               padding: 5px 8px; min-height: 28px; }"
        "QComboBox:focus, QSpinBox:focus, QLineEdit:focus { border: 1px solid #2563eb; }"
        "QAbstractSpinBox { color: #111827; selection-color: #111827;"
        "                  selection-background-color: #dbeafe; }"
        "QTableWidget { color: #111827; background: #ffffff; border: 1px solid #94a3b8;"
        "               border-radius: 8px; gridline-color: #e2e8f0;"
        "               selection-background-color: #bfdbfe; selection-color: #111827; }"
        "QTableWidget::item { color: #111827; }"
        "QHeaderView::section { background: #dbe4f0; color: #0f172a; border: 0;"
        "                       padding: 7px; font-weight: 700; }"
        "QLabel#sectionTitle { font-size: 15px; font-weight: 700; color: #0f172a; }");
}

QString dark_theme_style_sheet() {
    return QStringLiteral(
        "QMainWindow { background-color: #0f172a; }"
        "QLabel { color: #e2e8f0; font-size: 13px; }"
        "QPushButton { background-color: #3b82f6; color: #f8fafc; border: 0; border-radius: 8px;"
        "              padding: 7px 12px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:pressed { background-color: #1d4ed8; }"
        "QPushButton#themeButton { background-color: #475569; }"
        "QPushButton#themeButton:hover { background-color: #334155; }"
        "QComboBox, QSpinBox, QLineEdit { color: #f8fafc; background: #1e293b;"
        "                               border: 1px solid #64748b; border-radius: 6px;"
        "                               padding: 5px 8px; min-height: 28px; }"
        "QComboBox:focus, QSpinBox:focus, QLineEdit:focus { border: 1px solid #60a5fa; }"
        "QAbstractSpinBox { color: #f8fafc; selection-color: #f8fafc;"
        "                  selection-background-color: #334155; }"
        "QTableWidget { color: #f8fafc; background: #111827; border: 1px solid #64748b;"
        "               border-radius: 8px; gridline-color: #334155;"
        "               selection-background-color: #1d4ed8; selection-color: #f8fafc; }"
        "QTableWidget::item { color: #f8fafc; }"
        "QHeaderView::section { background: #334155; color: #f8fafc; border: 0;"
        "                       padding: 7px; font-weight: 700; }"
        "QLabel#sectionTitle { font-size: 15px; font-weight: 700; color: #f8fafc; }");
}

}  // namespace

class GanttWidget : public QWidget {
public:
    explicit GanttWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(120);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void set_gantt(const std::vector<int>& gantt, int max_pid_hint) {
        gantt_ = gantt;
        max_pid_ = max_pid_hint;
        if (max_pid_ <= 0) {
            for (int x : gantt_)
                if (x > max_pid_) max_pid_ = x;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), palette().color(QPalette::Base));

        const int n = static_cast<int>(gantt_.size());
        if (n == 0) {
            p.setPen(palette().color(QPalette::PlaceholderText));
            p.drawText(rect(), Qt::AlignCenter, QStringLiteral("No schedule"));
            return;
        }

        const double w = static_cast<double>(width());
        const double slice_w = w / static_cast<double>(n);
        const int row_top = 24;
        const int row_h = height() - row_top - 28;

        p.setPen(Qt::NoPen);
        for (int i = 0; i < n; ++i) {
            const int pid = gantt_[static_cast<size_t>(i)];
            QColor fill =
                (pid < 0) ? QColor(220, 220, 225) : color_for_pid(pid, std::max(1, max_pid_));
            const double x0 = slice_w * static_cast<double>(i);
            const double x1 = slice_w * static_cast<double>(i + 1);
            p.setBrush(fill);
            p.drawRect(static_cast<int>(std::floor(x0)), row_top,
                       std::max(1, static_cast<int>(std::ceil(x1 - x0))), row_h);
        }

        p.setPen(palette().color(QPalette::Text));
        p.setBrush(Qt::NoBrush);
        const int tick_every = std::max(1, n / 40);
        for (int i = 0; i <= n; i += tick_every) {
            const double x = slice_w * static_cast<double>(i);
            p.drawLine(static_cast<int>(std::round(x)), row_top + row_h,
                       static_cast<int>(std::round(x)), row_top + row_h + 6);
            if (i % (tick_every * 5) == 0 || i == n)
                p.drawText(static_cast<int>(std::round(x)) - 8, height() - 4,
                           QString::number(i));
        }

        p.drawText(4, 16, QStringLiteral("Timeline"));
    }

private:
    std::vector<int> gantt_;
    int max_pid_{};
};

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("CPU Scheduling Simulator"));
    resize(960, 640);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto* controls = new QHBoxLayout();
    controls->setSpacing(10);
    controls->addWidget(new QLabel(tr("Number of Processes:"), this));
    spin_count_ = new QSpinBox(this);
    spin_count_->setRange(1, 32);
    spin_count_->setValue(4);
    controls->addWidget(spin_count_);

    controls->addSpacing(16);
    controls->addWidget(new QLabel(tr("Scheduling Algorithm:"), this));
    combo_algo_ = new QComboBox(this);
    combo_algo_->addItem(tr("First Come First Served"));
    combo_algo_->addItem(tr("Shortest Job First (Non Preemptive)"));
    combo_algo_->addItem(tr("Round Robin"));
    combo_algo_->addItem(tr("Shortest Remaining Time First"));
    combo_algo_->addItem(tr("Priority Scheduling (Non Preemptive)"));
    combo_algo_->addItem(tr("Compare All Algorithms (Except Round Robin)"));
    controls->addWidget(combo_algo_);

    label_quantum_ = new QLabel(tr("Time Quantum:"), this);
    controls->addWidget(label_quantum_);
    spin_quantum_ = new QSpinBox(this);
    spin_quantum_->setRange(1, 100);
    spin_quantum_->setValue(2);
    controls->addWidget(spin_quantum_);

    controls->addStretch();
    btn_random_ = new QPushButton(tr("Generate Random Input"), this);
    btn_run_ = new QPushButton(tr("Run Simulation"), this);
    btn_theme_ = new QPushButton(tr("Switch to Dark Theme"), this);
    btn_theme_->setObjectName(QStringLiteral("themeButton"));
    controls->addWidget(btn_random_);
    controls->addWidget(btn_run_);
    controls->addWidget(btn_theme_);
    root->addLayout(controls);

    auto* input_title = new QLabel(tr("Process Input"), this);
    input_title->setObjectName(QStringLiteral("sectionTitle"));
    root->addWidget(input_title);

    table_input_ = new QTableWidget(this);
    table_input_->setAlternatingRowColors(true);
    root->addWidget(table_input_, 1);

    stack_results_ = new QStackedWidget(this);

    page_single_ = new QWidget(this);
    auto* single_lay = new QVBoxLayout(page_single_);
    table_results_ = new QTableWidget(this);
    table_results_->setAlternatingRowColors(true);
    single_lay->addWidget(table_results_);
    label_avg_ = new QLabel(this);
    single_lay->addWidget(label_avg_);
    gantt_ = new GanttWidget(this);
    single_lay->addWidget(gantt_);

    page_compare_ = new QWidget(this);
    auto* cmp_lay = new QVBoxLayout(page_compare_);
    table_compare_ = new QTableWidget(this);
    table_compare_->setAlternatingRowColors(true);
    cmp_lay->addWidget(table_compare_);
    cmp_lay->addWidget(
        new QLabel(tr("Run opens another window with one Gantt chart tab per algorithm."), this));

    stack_results_->addWidget(page_single_);
    stack_results_->addWidget(page_compare_);

    auto* results_title = new QLabel(tr("Simulation Results"), this);
    results_title->setObjectName(QStringLiteral("sectionTitle"));
    root->addWidget(results_title);
    root->addWidget(stack_results_, 1);

    setup_tables();

    connect(spin_count_, qOverload<int>(&QSpinBox::valueChanged), this,
            &MainWindow::on_process_count_changed);
    connect(combo_algo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::on_algorithm_changed);
    connect(btn_random_, &QPushButton::clicked, this, &MainWindow::on_random_clicked);
    connect(btn_run_, &QPushButton::clicked, this, &MainWindow::on_run_clicked);
    connect(btn_theme_, &QPushButton::clicked, this, &MainWindow::on_theme_toggle_clicked);

    on_process_count_changed(spin_count_->value());
    on_algorithm_changed(combo_algo_->currentIndex());
    apply_theme(false);
}

void MainWindow::apply_theme(bool dark_mode) {
    dark_mode_enabled_ = dark_mode;
    setStyleSheet(dark_mode_enabled_ ? dark_theme_style_sheet() : light_theme_style_sheet());
    btn_theme_->setText(dark_mode_enabled_ ? tr("Switch to Light Theme")
                                           : tr("Switch to Dark Theme"));
}

void MainWindow::on_theme_toggle_clicked() { apply_theme(!dark_mode_enabled_); }

void MainWindow::setup_tables() {
    QStringList in_headers{QStringLiteral("Process ID"), QStringLiteral("Arrival Time"),
                           QStringLiteral("Burst Time"), QStringLiteral("Priority Level")};
    table_input_->setColumnCount(in_headers.size());
    table_input_->setHorizontalHeaderLabels(in_headers);
    table_input_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_input_->verticalHeader()->setVisible(false);
    table_input_->setSelectionBehavior(QAbstractItemView::SelectItems);
    table_input_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed |
                                  QAbstractItemView::SelectedClicked);

    QStringList out_headers{QStringLiteral("Process ID"), QStringLiteral("Arrival Time"),
                            QStringLiteral("Burst Time"), QStringLiteral("Completion Time"),
                            QStringLiteral("Waiting Time"), QStringLiteral("Turnaround Time")};
    table_results_->setColumnCount(out_headers.size());
    table_results_->setHorizontalHeaderLabels(out_headers);
    table_results_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_results_->verticalHeader()->setVisible(false);
    table_results_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_results_->setSelectionBehavior(QAbstractItemView::SelectRows);

    table_compare_->setColumnCount(3);
    table_compare_->setHorizontalHeaderLabels(
        {QStringLiteral("Scheduling Algorithm"), QStringLiteral("Average Waiting Time"),
         QStringLiteral("Average Turnaround Time")});
    table_compare_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_compare_->verticalHeader()->setVisible(false);
    table_compare_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_compare_->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::on_process_count_changed(int n) {
    table_input_->setRowCount(n);
    for (int i = 0; i < n; ++i) {
        QTableWidgetItem* pid_item = nullptr;
        if (!table_input_->item(i, 0)) {
            pid_item = new QTableWidgetItem(QString::number(i + 1));
            table_input_->setItem(i, 0, pid_item);
        } else {
            pid_item = table_input_->item(i, 0);
            pid_item->setText(QString::number(i + 1));
        }
        pid_item->setFlags((pid_item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled) &
                           ~Qt::ItemIsEditable);

        for (int c = 1; c < 4; ++c) {
            if (!table_input_->item(i, c)) {
                const QString z = QStringLiteral("0");
                table_input_->setItem(i, c, new QTableWidgetItem(c == 2 ? QStringLiteral("1") : z));
            }
        }
    }
}

void MainWindow::on_algorithm_changed(int index) {
    const bool rr = (index == 2);
    const bool compare = (index == k_algo_compare_index);
    label_quantum_->setVisible(rr && !compare);
    spin_quantum_->setVisible(rr && !compare);

    if (compare)
        stack_results_->setCurrentWidget(page_compare_);
    else
        stack_results_->setCurrentWidget(page_single_);
}

void MainWindow::on_random_clicked() {
    const int n = spin_count_->value();
    std::vector<Process> procs = generate_random_processes(n);
    for (int i = 0; i < n; ++i) {
        const Process& pr = procs[static_cast<size_t>(i)];
        table_input_->item(i, 0)->setText(QString::number(pr.pid));
        table_input_->item(i, 1)->setText(QString::number(pr.at));
        table_input_->item(i, 2)->setText(QString::number(pr.bt));
        table_input_->item(i, 3)->setText(QString::number(pr.priority));
    }
}

std::vector<Process> MainWindow::collect_processes_from_table() {
    const int n = table_input_->rowCount();
    std::vector<Process> out;
    out.reserve(static_cast<size_t>(n));

    for (int i = 0; i < n; ++i) {
        Process pr;
        pr.pid = i + 1;

        auto read_int = [&](int col, const char* field_name) -> std::optional<int> {
            auto* it = table_input_->item(i, col);
            if (!it || it->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, tr("Input"),
                                     tr("Missing %1 for row %2").arg(QString::fromUtf8(field_name)).arg(i + 1));
                return std::nullopt;
            }
            bool ok = false;
            const int v = it->text().trimmed().toInt(&ok);
            if (!ok) {
                QMessageBox::warning(this, tr("Input"),
                                     tr("Invalid %1 for row %2").arg(QString::fromUtf8(field_name)).arg(i + 1));
                return std::nullopt;
            }
            return v;
        };

        if (auto at = read_int(1, "arrival"))
            pr.at = *at;
        else
            return {};

        if (auto bt = read_int(2, "burst")) {
            if (*bt <= 0) {
                QMessageBox::warning(this, tr("Input"),
                                     tr("Burst time must be positive (row %1).").arg(i + 1));
                return {};
            }
            pr.bt = *bt;
        } else
            return {};

        if (auto pr_val = read_int(3, "priority"))
            pr.priority = *pr_val;
        else
            return {};

        pr.rt = 0;
        pr.ct = 0;
        pr.tat = 0;
        pr.wt = 0;
        out.push_back(pr);
    }
    return out;
}

void MainWindow::fill_comparison_view(const std::vector<Process>& original) {
    struct Row {
        QString name;
        float wt;
        float tat;
        std::vector<int> gantt;
        int max_pid{};
    };
    std::vector<Row> rows;

    auto max_pid_of = [](const std::vector<Process>& v) {
        int m = 0;
        for (const auto& p : v) m = std::max(m, p.pid);
        return m;
    };

    {
        auto r = schedule_fcfs(original);
        rows.push_back(
            {QStringLiteral("First Come First Served"), r.avg_wt, r.avg_tat, r.gantt, max_pid_of(r.processes)});
    }
    {
        auto r = schedule_sjf(original);
        rows.push_back({QStringLiteral("Shortest Job First"), r.avg_wt, r.avg_tat, r.gantt,
                        max_pid_of(r.processes)});
    }
    {
        auto r = schedule_srtf(original);
        rows.push_back({QStringLiteral("Shortest Remaining Time First"), r.avg_wt, r.avg_tat, r.gantt,
                        max_pid_of(r.processes)});
    }
    {
        auto r = schedule_priority(original);
        rows.push_back(
            {QStringLiteral("Priority Scheduling"), r.avg_wt, r.avg_tat, r.gantt,
             max_pid_of(r.processes)});
    }

    table_compare_->setRowCount(static_cast<int>(rows.size()));
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        table_compare_->setItem(i, 0, new QTableWidgetItem(rows[static_cast<size_t>(i)].name));
        table_compare_->setItem(
            i, 1, new QTableWidgetItem(QString::number(rows[static_cast<size_t>(i)].wt, 'f', 2)));
        table_compare_->setItem(
            i, 2, new QTableWidgetItem(QString::number(rows[static_cast<size_t>(i)].tat, 'f', 2)));
    }

    stack_results_->setCurrentWidget(page_compare_);

    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Gantt Charts Comparison"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->resize(900, 420);
    auto* vl = new QVBoxLayout(dlg);
    auto* tabs = new QTabWidget(dlg);
    vl->addWidget(tabs);

    for (const Row& row : rows) {
        auto* page = new QWidget(tabs);
        auto* pl = new QVBoxLayout(page);
        auto* gw = new GanttWidget(page);
        gw->set_gantt(row.gantt, row.max_pid);
        pl->addWidget(gw);
        tabs->addTab(page, row.name);
    }
    dlg->show();
}

void MainWindow::on_run_clicked() {
    std::vector<Process> procs = collect_processes_from_table();
    if (procs.empty()) return;

    const int algo_index = combo_algo_->currentIndex();

    if (algo_index == k_algo_compare_index) {
        fill_comparison_view(procs);
        return;
    }

    Algorithm algo = Algorithm::FCFS;
    int quantum = 2;
    switch (algo_index) {
        case 0:
            algo = Algorithm::FCFS;
            break;
        case 1:
            algo = Algorithm::SJF;
            break;
        case 2:
            algo = Algorithm::RR;
            quantum = spin_quantum_->value();
            break;
        case 3:
            algo = Algorithm::SRTF;
            break;
        case 4:
            algo = Algorithm::Priority;
            break;
        default:
            algo = Algorithm::FCFS;
            break;
    }

    ScheduleResult res = run_scheduler(std::move(procs), algo, quantum);

    table_results_->setRowCount(static_cast<int>(res.processes.size()));
    int max_pid = 0;
    for (int i = 0; i < static_cast<int>(res.processes.size()); ++i) {
        const Process& pr = res.processes[static_cast<size_t>(i)];
        max_pid = std::max(max_pid, pr.pid);
        table_results_->setItem(i, 0, new QTableWidgetItem(QString::number(pr.pid)));
        table_results_->setItem(i, 1, new QTableWidgetItem(QString::number(pr.at)));
        table_results_->setItem(i, 2, new QTableWidgetItem(QString::number(pr.bt)));
        table_results_->setItem(i, 3, new QTableWidgetItem(QString::number(pr.ct)));
        table_results_->setItem(i, 4, new QTableWidgetItem(QString::number(pr.wt)));
        table_results_->setItem(i, 5, new QTableWidgetItem(QString::number(pr.tat)));
    }

    label_avg_->setText(tr("Average Waiting Time: %1, Average Turnaround Time: %2")
                            .arg(QString::number(res.avg_wt, 'f', 2))
                            .arg(QString::number(res.avg_tat, 'f', 2)));

    gantt_->set_gantt(res.gantt, max_pid);

    stack_results_->setCurrentWidget(page_single_);
}
