#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include <random>

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
	AssembleProgram( m_programID, "Shaders/Vert_PosCol.vert", "Shaders/Frag_PosCol.frag" );

}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
}

void CMyApp::InitGeometry()
{
	MeshObject<VertexPosColor> meshCPU;

	constexpr int pointCount = 64;
	meshCPU.vertexArray.push_back({ glm::vec3(0, 0, 0), {0.25, 0.50, 0.75} });


	//First circle, on the XY plane
	for (int i = 0; i < pointCount; i++) {

		glm::vec3 pos = glm::vec3(cosf((i / (float)pointCount) * glm::two_pi<float>()),
			sinf((i / (float)pointCount) * glm::two_pi<float>()),
			0.f);

		glm::vec3 col = pos * 0.5f + 0.5f; //so their colours are different, always

		meshCPU.vertexArray.push_back({ pos, col });
	}

	for (int i = 0; i < pointCount - 1; i++) {
		meshCPU.indexArray.push_back(i + 2);
		meshCPU.indexArray.push_back(i + 1);
		meshCPU.indexArray.push_back(0);
	}
	meshCPU.indexArray.push_back(1);
	meshCPU.indexArray.push_back(pointCount);
	meshCPU.indexArray.push_back(0);

	//first circle set up

	//second circle, 3 above the XY plane
	for (int i = 0; i < pointCount; i++) {

		glm::vec3 pos = glm::vec3(cosf((i / (float)pointCount) * glm::two_pi<float>()),
			sinf((i / (float)pointCount) * glm::two_pi<float>()),
			3.f);

		glm::vec3 col = pos * 0.5f + 0.5f; //so their colours are different, always

		meshCPU.vertexArray.push_back({ pos, col });
	}

	for (int i = 65; i < (pointCount * 2) - 1; i++) {

		meshCPU.indexArray.push_back(pointCount + 1);
		meshCPU.indexArray.push_back(i + 1);
		meshCPU.indexArray.push_back(i + 2);
	}
	meshCPU.indexArray.push_back(pointCount + 1);
	meshCPU.indexArray.push_back(pointCount * 2);
	meshCPU.indexArray.push_back(pointCount + 2);

	//second circle is complete, now to connect them, first, bottom to top

	for (int i = 0; i < pointCount; i++) {

		meshCPU.indexArray.push_back(i);
		meshCPU.indexArray.push_back(i + 1);
		meshCPU.indexArray.push_back(i + pointCount);
	}
	meshCPU.indexArray.push_back(1);
	meshCPU.indexArray.push_back(pointCount * 2);
	meshCPU.indexArray.push_back(pointCount);

	//now, the top to bottom vertices

	for (int i = 65; i < pointCount * 2; i++) {

		meshCPU.indexArray.push_back(i - pointCount + 1);
		meshCPU.indexArray.push_back(i + 1);
		meshCPU.indexArray.push_back(i);
	}

	meshCPU.indexArray.push_back(pointCount * 2);
	meshCPU.indexArray.push_back(1);
	meshCPU.indexArray.push_back(pointCount + 2);

	//this should be a separate function, I'll see if I get around to it
	//but hey, this isn't going to production!


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

	glEnableVertexAttribArray(1); // ez lesz majd a pozíció
	glVertexAttribPointer(
		1,						  // a VB-ben található adatok közül a 1. "indexű" attribútumait állítjuk be
		3,						  // komponens szam
		GL_FLOAT,				  // adatok tipusa
		GL_FALSE,				  // normalizalt legyen-e
		sizeof(VertexPosColor),   // stride (0=egymas utan)
		reinterpret_cast<const void*>(offsetof(VertexPosColor,color )) // a 1. indexű attribútum hol kezdődik a sizeof(Vertex)-nyi területen belül
	);

	// index puffer létrehozása
	glGenBuffers(1, &iboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
				  meshCPU.indexArray.size() * sizeof(GLuint), 
				  meshCPU.indexArray.data(), 
				  GL_STATIC_DRAW);

	count = static_cast<GLsizei>(meshCPU.indexArray.size());

	glBindVertexArray( 0 ); // Kapcsoljuk ki a VAO-t!
}

void CMyApp::CleanGeometry()
{
	glDeleteBuffers(1,      &vboID);
	glDeleteBuffers(1,      &iboID);
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

	GenerateCoordinates();

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását //TODO SET TO ENABLE, IF I FORGOT TO SET THIS TO ENABLE IM TERRIBLY SORRY
	//glPolygonMode(GL_BACK, GL_LINE);
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 0.0, 5.0),// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

	m_cameraManipulator.SetCamera( &m_camera );

	return true;
}

void CMyApp::GenerateCoordinates() 
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distr(-5.0f, 5.0f);
	int i = 0;
	while (i < 6)
	{
		float x = distr(gen);
		float y = distr(gen);
		float z = distr(gen);

		float term1 = powf(x, 3.0);
		float term2 = powf(y, 4.0);
		float term3 = x * y;
		float term4 = x * powf(z, 3.0);

		if ((term1 + term2 - term3 + term4) < 10)
		{
			displacementPoints.push_back(glm::vec3(x, y, z));
			i++;
		}
	}
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_cameraManipulator.Update( updateInfo.DeltaTimeInSec );
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - VAO beállítása
	glBindVertexArray( vaoID );

	// - shader bekapcsolasa
	glUseProgram( m_programID );

	// - Uniform paraméterek

	// view és projekciós mátrix
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// Transzformációs mátrixok
	/*

	GLM transzformációs mátrixokra példák:
		glm::rotate<float>( szög, glm::vec3(tengely_x, tengely_y, tengely_z) ) <- tengely_{xyz} körüli elforgatás
		glm::translate<float>( glm::vec3(eltol_x, eltol_y, eltol_z) ) <- eltolás
		glm::scale<float>( glm::vec3(s_x, s_y, s_z) ) <- skálázás

	*/
	

	glm::mat4 matWorld = glm::identity<glm::mat4>();
	glUniformMatrix4fv( ul("world"), 1, GL_FALSE, glm::value_ptr( matWorld ) );
	glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr );


	float a = m_ElapsedTimeInSec * glm::two_pi<float>() / 10.f;

	float b = rotation_state * glm::two_pi<float>() / 10.f;
	for (int i = 0; i < 6; i++) {
		glm::mat4 matWorld = glm::identity<glm::mat4>();

		if (space_pressed) {
			rotation_state = rotation_state + 0.05f;
		}

		matWorld = glm::rotate(0.5f / (float)6 * glm::three_over_two_pi<float>() + b, glm::vec3(0, 1, 0)) * matWorld;
		

		matWorld =
			glm::rotate(0.5f / (float)6 * glm::three_over_two_pi<float>() + a, glm::vec3(0, 1, 0))
			*
			glm::rotate(0.5f / (float)6 * glm::three_over_two_pi<float>() + a, glm::vec3(0, 0, 1))
			*
			glm::translate(glm::vec3(9, 0, 0))
			*
			matWorld;

		matWorld = glm::translate(displacementPoints[i]) * matWorld;

		glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	// shader kikapcsolasa
	glUseProgram( 0 );

	// VAO kikapcsolása
	glBindVertexArray( 0 );
}

void CMyApp::RenderGUI()
{
	// ImGui::ShowDemoWindow();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;

	// Kérdezzük le az aktuális programot!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );
	// A program és a uniform név ismeretében kérdezzük le a location-t!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
	return glGetUniformLocation( programID, uniformName );
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

		if (key.keysym.sym == SDLK_SPACE)
		{
			space_pressed = !space_pressed;
		}
	}
	m_cameraManipulator.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp( key );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove( mouse );
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
	m_cameraManipulator.MouseWheel( wheel );
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.SetAspect( static_cast<float>(_w) / _h );
}

// Le nem kezelt, egzotikus esemény kezelése
// https://wiki.libsdl.org/SDL2/SDL_Event

void CMyApp::OtherEvent( const SDL_Event& ev )
{

}