#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"

#include <imgui.h>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AssembleProgram( m_programID, "Shaders/Vert_PosCol_NoTransform.vert", "Shaders/Frag_PosCol.frag" );

}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
}

void CMyApp::InitGeometry()
{
	MeshObject<VertexPosColor> meshCPU;

	meshCPU.vertexArray = 
	{
		{ glm::vec3( 0, 1, 0), glm::vec3(1, 0, 0) }, //JA ITT A MÁSODIK AZ SZÍN?
		{ glm::vec3(-1, 1, 0), glm::vec3(1, 0, 0) },
		{ glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0) },

		{ glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0) },
		{ glm::vec3(-1, -1, 0), glm::vec3(0, 1, 0) },
		{ glm::vec3(0, -1, 0), glm::vec3(0, 1, 0) },

		{ glm::vec3(0, -1, 0), glm::vec3(0, 0, 1) },
		{ glm::vec3(1, -1, 0), glm::vec3(0, 0, 1) },
		{ glm::vec3(1, 0, 0), glm::vec3(0, 0, 1) },

		{ glm::vec3(1, 0, 0), glm::vec3(0.11, 0.99, 0.33) },
		{ glm::vec3(1, 1, 0), glm::vec3(0.54, 0.32, 0.76) },
		{ glm::vec3(0, 1, 0), glm::vec3(0.22, 0.64, 0.15) },
	
	};

	// 1 db VAO foglalasa
	glGenVertexArrays(1, &vaoID);
	// a frissen generált VAO beallitasa aktívnak
	glBindVertexArray(vaoID);

	// hozzunk létre egy új VBO erőforrás nevet
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID); // tegyük "aktívvá" a létrehozott VBO-t

	// töltsük fel adatokkal az aktív VBO-t
	glBufferData(GL_ARRAY_BUFFER,	// az aktív VBO-ba töltsünk adatokat
				  meshCPU.vertexArray.size() * sizeof(VertexPosColor),		// ennyi bájt nagyságban
				  meshCPU.vertexArray.data(),	// erről a rendszermemóriabeli címről olvasva
				  GL_STATIC_DRAW);	// úgy, hogy a VBO-nkba nem tervezünk ezután írni és minden kirajzoláskor felhasnzáljuk a benne lévő adatokat

	glEnableVertexAttribArray(0); // ez lesz majd a pozíció
	glVertexAttribPointer(
		0,						  // a VB-ben található adatok közül a 0. "indexű" attribútumait állítjuk be
		3,						  // komponens szam
		GL_FLOAT,				  // adatok tipusa
		GL_FALSE,				  // normalizalt legyen-e
		sizeof(VertexPosColor),   // stride (0=egymas utan)
		reinterpret_cast<const void*>(offsetof(VertexPosColor,position )) // a 0. indexű attribútum hol kezdődik a sizeof(Vertex)-nyi területen belül
	);

	glEnableVertexAttribArray(1); // ez lesz majd a szín
	glVertexAttribPointer(
		1,						  // a VB-ben található adatok közül a 1. "indexű" attribútumait állítjuk be
		3,						  // komponens szam
		GL_FLOAT,				  // adatok tipusa
		GL_FALSE,				  // normalizalt legyen-e
		sizeof(VertexPosColor),   // stride (0=egymas utan)
		reinterpret_cast<const void*>(offsetof(VertexPosColor,color )) // a 1. indexű attribútum hol kezdődik a sizeof(Vertex)-nyi területen belül
	);

	count = static_cast<GLsizei>(meshCPU.vertexArray.size());
	
	glBindVertexArray( 0 ); // Kapcsoljuk ki a VAO-t!
}

void CMyApp::CleanGeometry()
{
	glDeleteBuffers(1,      &vboID);
	glDeleteVertexArrays(1, &vaoID);
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - VAO beállítása
	glBindVertexArray( vaoID );

	// shader bekapcsolasa
	glUseProgram( m_programID );

	// kirajzolás: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArrays.xhtml
	glDrawArrays( GL_TRIANGLES,	// primitív típusa; amikkel mi foglalkozunk: GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES
				  0,			// melyik vertex az elso
				  count );		// hány csúcspontot használjunk a primitívek kirajzolására

	// shader kikapcsolasa
	glUseProgram( 0 );

	// VAO kikapcsolása
	glBindVertexArray( 0 );
}

void CMyApp::RenderGUI()
{
	// ImGui::ShowDemoWindow();
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) // Először lett megnyomva
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); // Állítsuk be az újat!
		}
	}
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{

}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
}

// Le nem kezelt, egzotikus esemény kezelése
// https://wiki.libsdl.org/SDL2/SDL_Event

void CMyApp::OtherEvent( const SDL_Event& ev )
{

}