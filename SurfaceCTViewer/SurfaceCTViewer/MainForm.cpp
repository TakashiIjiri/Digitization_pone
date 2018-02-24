#include "stdafx.h"
#include "MainForm.h"
#include "TCore.h"


using namespace SurfaceCTViewer;


MainForm::MainForm()
{
	printf("MainForm constructor\n");
	m_ogl = 0;
	InitializeComponent();

	m_prevKeyID = -1;


	m_ogl = new OglForCLI(GetDC((HWND)m_mainPanel->Handle.ToPointer()));
	
	EVec3f cuboid(10,10,10);
	EVec3f camC = cuboid * 0.5;
	EVec3f camY(0, -1, 0);
	EVec3f camP = camC - cuboid[2] * 1.5f * EVec3f(0, 0, 1);
	m_ogl->SetCam(camP, camC, camY);

	m_ogl->SetBgColor(0.1f, 0.1f, 0.1f, 0.5f);

	printf("MainForm constructor -- done\n");

}




void MainForm::RedrawMainPanel()
{
	if( m_ogl == 0 ) return;


	m_ogl->SetBgColor(1,1,1,1);
	EVec3f cuboid(10,10,10);//ImageCore::getInst()->getCuboidF();
	float  nearDist = (cuboid[0] + cuboid[1] + cuboid[2]) / 3.0f * 0.01f;
	float  farDist = (cuboid[0] + cuboid[1] + cuboid[2]) / 3.0f * 16;
	m_ogl->OnDrawBegin( m_mainPanel->Width, m_mainPanel->Height, 45.0, nearDist, farDist);
	
	EVec3f p = m_ogl->GetCamPos();
	TCore::getInst()->drawScene( EVec3d( (double)p[0], (double)p[0], (double)p[0]));
	m_ogl->OnDrawEnd();
}


void MainForm::GetProjModelViewMat(double *model, double *proj, int*vp)
{
		
	EVec3f cuboid(10,10,10);//ImageCore::getInst()->getCuboidF();
	float  nearDist = (cuboid[0] + cuboid[1] + cuboid[2]) / 3.0f * 0.01f;
	float  farDist = (cuboid[0] + cuboid[1] + cuboid[2]) / 3.0f * 16;
	m_ogl->getProjModelViewMats( model, proj, vp, m_mainPanel->Width, m_mainPanel->Height, 45.0, nearDist, farDist);
}

System::Void MainForm::m_mainPanel_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if (e->Button == System::Windows::Forms::MouseButtons::Left  ) TCore::getInst()->BtnDownL( EVec2i(e->X, e->Y), m_ogl);
	if (e->Button == System::Windows::Forms::MouseButtons::Middle) TCore::getInst()->BtnDownM( EVec2i(e->X, e->Y), m_ogl);
	if (e->Button == System::Windows::Forms::MouseButtons::Right ) TCore::getInst()->BtnDownR( EVec2i(e->X, e->Y), m_ogl);
}

System::Void MainForm::m_mainPanel_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if (e->Button == System::Windows::Forms::MouseButtons::Left  ) TCore::getInst()->BtnUpL( EVec2i(e->X, e->Y), m_ogl );
	if (e->Button == System::Windows::Forms::MouseButtons::Middle) TCore::getInst()->BtnUpM( EVec2i(e->X, e->Y), m_ogl );
	if (e->Button == System::Windows::Forms::MouseButtons::Right ) TCore::getInst()->BtnUpR( EVec2i(e->X, e->Y), m_ogl );
}

System::Void MainForm::m_mainPanel_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	TCore::getInst()->MouseMove( EVec2i(e->X, e->Y), m_ogl );
}

