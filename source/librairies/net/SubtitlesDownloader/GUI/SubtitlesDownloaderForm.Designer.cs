namespace SubtitlesDownloader
{
    partial class SubtitlesDownloaderForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.errorMenuStripLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.helloToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.displayErrorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusBarToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel2 = new System.Windows.Forms.Panel();
            this.resultPanel = new System.Windows.Forms.Panel();
            this.resultFromSearchGridView = new System.Windows.Forms.DataGridView();
            this.TitleColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.HostNameColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DescriptionColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.textPanel = new System.Windows.Forms.Panel();
            this.filenameTextBox = new System.Windows.Forms.TextBox();
            this.errorPanel = new System.Windows.Forms.Panel();
            this.errorDataGridView = new System.Windows.Forms.DataGridView();
            this.TimestampColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ErrorMessageColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.statusStrip.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.resultPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.resultFromSearchGridView)).BeginInit();
            this.textPanel.SuspendLayout();
            this.errorPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.errorDataGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.errorMenuStripLabel});
            this.statusStrip.Location = new System.Drawing.Point(0, 362);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(955, 22);
            this.statusStrip.TabIndex = 10;
            this.statusStrip.Text = "statusStrip";
            // 
            // errorMenuStripLabel
            // 
            this.errorMenuStripLabel.Enabled = false;
            this.errorMenuStripLabel.Image = global::SubtitlesDownloader.Properties.Resources.error;
            this.errorMenuStripLabel.Name = "errorMenuStripLabel";
            this.errorMenuStripLabel.Size = new System.Drawing.Size(62, 17);
            this.errorMenuStripLabel.Text = "0 Errors";
            this.errorMenuStripLabel.Click += new System.EventHandler(this.DisplayErrorPanel);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.helloToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(955, 24);
            this.menuStrip1.TabIndex = 3;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // helloToolStripMenuItem
            // 
            this.helloToolStripMenuItem.Name = "helloToolStripMenuItem";
            this.helloToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.helloToolStripMenuItem.Text = "Hello";
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.displayErrorToolStripMenuItem,
            this.statusBarToolStripMenuItem});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.viewToolStripMenuItem.Text = "&View";
            // 
            // displayErrorToolStripMenuItem
            // 
            this.displayErrorToolStripMenuItem.Name = "displayErrorToolStripMenuItem";
            this.displayErrorToolStripMenuItem.Size = new System.Drawing.Size(126, 22);
            this.displayErrorToolStripMenuItem.Text = "Error";
            this.displayErrorToolStripMenuItem.Click += new System.EventHandler(this.DisplayErrorPanel);
            // 
            // statusBarToolStripMenuItem
            // 
            this.statusBarToolStripMenuItem.Checked = true;
            this.statusBarToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.statusBarToolStripMenuItem.Name = "statusBarToolStripMenuItem";
            this.statusBarToolStripMenuItem.Size = new System.Drawing.Size(126, 22);
            this.statusBarToolStripMenuItem.Text = "Status Bar";
            this.statusBarToolStripMenuItem.Click += new System.EventHandler(this.statusBarToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "&Help";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.panel1, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.panel2, 1, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(955, 338);
            this.tableLayoutPanel1.TabIndex = 11;
            // 
            // panel1
            // 
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(24, 332);
            this.panel1.TabIndex = 0;
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.resultPanel);
            this.panel2.Controls.Add(this.textPanel);
            this.panel2.Controls.Add(this.errorPanel);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel2.Location = new System.Drawing.Point(33, 3);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(919, 332);
            this.panel2.TabIndex = 1;
            // 
            // resultPanel
            // 
            this.resultPanel.Controls.Add(this.resultFromSearchGridView);
            this.resultPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.resultPanel.Location = new System.Drawing.Point(0, 25);
            this.resultPanel.Name = "resultPanel";
            this.resultPanel.Size = new System.Drawing.Size(919, 217);
            this.resultPanel.TabIndex = 1;
            // 
            // resultFromSearchGridView
            // 
            this.resultFromSearchGridView.AllowUserToAddRows = false;
            this.resultFromSearchGridView.AllowUserToDeleteRows = false;
            this.resultFromSearchGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.resultFromSearchGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.TitleColumn,
            this.HostNameColumn,
            this.DescriptionColumn});
            this.resultFromSearchGridView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.resultFromSearchGridView.Location = new System.Drawing.Point(0, 0);
            this.resultFromSearchGridView.Name = "resultFromSearchGridView";
            this.resultFromSearchGridView.ReadOnly = true;
            this.resultFromSearchGridView.RowHeadersVisible = false;
            this.resultFromSearchGridView.Size = new System.Drawing.Size(919, 217);
            this.resultFromSearchGridView.TabIndex = 10;
            // 
            // TitleColumn
            // 
            this.TitleColumn.HeaderText = "Title";
            this.TitleColumn.Name = "TitleColumn";
            this.TitleColumn.ReadOnly = true;
            this.TitleColumn.Width = 250;
            // 
            // HostNameColumn
            // 
            this.HostNameColumn.HeaderText = "HostName";
            this.HostNameColumn.Name = "HostNameColumn";
            this.HostNameColumn.ReadOnly = true;
            this.HostNameColumn.Width = 200;
            // 
            // DescriptionColumn
            // 
            this.DescriptionColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.DescriptionColumn.HeaderText = "Description";
            this.DescriptionColumn.Name = "DescriptionColumn";
            this.DescriptionColumn.ReadOnly = true;
            // 
            // textPanel
            // 
            this.textPanel.Controls.Add(this.filenameTextBox);
            this.textPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.textPanel.Location = new System.Drawing.Point(0, 0);
            this.textPanel.Name = "textPanel";
            this.textPanel.Size = new System.Drawing.Size(919, 25);
            this.textPanel.TabIndex = 0;
            // 
            // filenameTextBox
            // 
            this.filenameTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.filenameTextBox.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.filenameTextBox.Location = new System.Drawing.Point(0, 0);
            this.filenameTextBox.Name = "filenameTextBox";
            this.filenameTextBox.Size = new System.Drawing.Size(919, 27);
            this.filenameTextBox.TabIndex = 8;
            // 
            // errorPanel
            // 
            this.errorPanel.Controls.Add(this.errorDataGridView);
            this.errorPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.errorPanel.Location = new System.Drawing.Point(0, 242);
            this.errorPanel.Name = "errorPanel";
            this.errorPanel.Size = new System.Drawing.Size(919, 90);
            this.errorPanel.TabIndex = 2;
            // 
            // errorDataGridView
            // 
            this.errorDataGridView.AllowUserToAddRows = false;
            this.errorDataGridView.AllowUserToDeleteRows = false;
            this.errorDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.errorDataGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.TimestampColumn,
            this.ErrorMessageColumn});
            this.errorDataGridView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.errorDataGridView.Location = new System.Drawing.Point(0, 0);
            this.errorDataGridView.Name = "errorDataGridView";
            this.errorDataGridView.ReadOnly = true;
            this.errorDataGridView.RowHeadersVisible = false;
            this.errorDataGridView.Size = new System.Drawing.Size(919, 90);
            this.errorDataGridView.TabIndex = 0;
            // 
            // TimestampColumn
            // 
            this.TimestampColumn.HeaderText = "Timestamp";
            this.TimestampColumn.Name = "TimestampColumn";
            this.TimestampColumn.ReadOnly = true;
            this.TimestampColumn.Width = 200;
            // 
            // ErrorMessageColumn
            // 
            this.ErrorMessageColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.ErrorMessageColumn.HeaderText = "Error Message";
            this.ErrorMessageColumn.Name = "ErrorMessageColumn";
            this.ErrorMessageColumn.ReadOnly = true;
            // 
            // SubtitlesDownloaderForm
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(955, 384);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.menuStrip1);
            this.Name = "SubtitlesDownloaderForm";
            this.Text = "Subtitle Downloader";
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.resultPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.resultFromSearchGridView)).EndInit();
            this.textPanel.ResumeLayout(false);
            this.textPanel.PerformLayout();
            this.errorPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.errorDataGridView)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem helloToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel errorMenuStripLabel;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem displayErrorToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem statusBarToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Panel textPanel;
        private System.Windows.Forms.TextBox filenameTextBox;
        private System.Windows.Forms.Panel resultPanel;
        private System.Windows.Forms.DataGridView resultFromSearchGridView;
        private System.Windows.Forms.Panel errorPanel;
        private System.Windows.Forms.DataGridView errorDataGridView;
        private System.Windows.Forms.DataGridViewTextBoxColumn TimestampColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn ErrorMessageColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn TitleColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn HostNameColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn DescriptionColumn;
    }
}

