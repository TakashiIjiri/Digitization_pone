#pragma once

#include "./COMMON/OglForCLI.h"
#include "TCore.h"



namespace SurfaceCTViewer {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// MainForm の概要
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{

	public:
		void RedrawMainPanel();
		void GetProjModelViewMat(double *model, double *proj, int*vp);
	private:
		OglForCLI *m_ogl;
		int m_prevKeyID;

	private:
		MainForm(void);

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  m_mainPanel;
	protected:

	protected:

		

	private:
		static MainForm^ m_singleton;

	public:
		static MainForm^ getInst() {
			if (m_singleton == nullptr) {
				m_singleton = gcnew MainForm();
			}
			return m_singleton;
		}




	private:
		/// <summary>
		/// 必要なデザイナー変数です。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナー サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディターで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->m_mainPanel = (gcnew System::Windows::Forms::Panel());
			this->SuspendLayout();
			// 
			// m_mainPanel
			// 
			this->m_mainPanel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->m_mainPanel->Location = System::Drawing::Point(0, 0);
			this->m_mainPanel->Margin = System::Windows::Forms::Padding(2);
			this->m_mainPanel->Name = L"m_mainPanel";
			this->m_mainPanel->Size = System::Drawing::Size(630, 594);
			this->m_mainPanel->TabIndex = 0;
			this->m_mainPanel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::m_mainPanel_Paint);
			this->m_mainPanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::m_mainPanel_MouseDown);
			this->m_mainPanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::m_mainPanel_MouseMove);
			this->m_mainPanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::m_mainPanel_MouseUp);
			this->m_mainPanel->Resize += gcnew System::EventHandler(this, &MainForm::m_mainPanel_Resize);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(630, 594);
			this->Controls->Add(this->m_mainPanel);
			this->Margin = System::Windows::Forms::Padding(2);
			this->Name = L"MainForm";
			this->Text = L"MainForm";
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::MainForm_Paint);
			this->Resize += gcnew System::EventHandler(this, &MainForm::MainForm_Resize);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void MainForm_Resize(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void MainForm_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
	}

	private: System::Void m_mainPanel_Resize(System::Object^  sender, System::EventArgs^  e) {
		RedrawMainPanel();
	}

	private: System::Void m_mainPanel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		RedrawMainPanel();
	}

	private: System::Void m_mainPanel_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e); 
	private: System::Void m_mainPanel_MouseUp  (System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e); 
	private: System::Void m_mainPanel_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
};


	inline void MainForm_redrawMainPanel()
	{
		MainForm::getInst()->RedrawMainPanel();
	}

		
	inline bool myFileDialog( const char* filter, std::string &fname)
	{
		OpenFileDialog^ dlg = gcnew OpenFileDialog();
		dlg->Filter   = gcnew System::String( filter );
		dlg->ShowHelp = true;

		if (dlg->ShowDialog() == System::Windows::Forms::DialogResult::Cancel) return false;

		IntPtr mptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(dlg->FileName);
		fname = static_cast<const char*>(mptr.ToPointer());

		return true;
	}

	inline void MainForm_GetProjModelViewMat(double *model, double *proj, int *vp)
	{
		MainForm::getInst()->GetProjModelViewMat( model, proj, vp);
	}

	inline bool isCtrKeyOn  () { return GetKeyState(VK_CONTROL) < 0; }
	inline bool isSpaceKeyOn() { return GetKeyState(VK_SPACE  ) < 0; }
	inline bool isShiftKeyOn() { return GetKeyState(VK_SHIFT  ) < 0; }
	inline bool isAltKeyOn  () { return GetKeyState(VK_MENU   ) < 0; }



}
