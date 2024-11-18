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
	AssembleProgram( m_programID, "Shaders/Vert_PosTex.vert", "Shaders/Frag_PosTex.frag" );

}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
}

void CMyApp::InitGeometry()
{
	// Pyramid 

	MeshObject<VertexPosTex> pyramidMeshCPU;

	static constexpr float SQRT_2 = glm::root_two<float>();

	pyramidMeshCPU.vertexArray = 
	{
		{ glm::vec3(-1.0,    0.0, -1.0), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3( 1.0,    0.0, -1.0), glm::vec2( 1.0, 0.0 ) },
		{ glm::vec3(-1.0,    0.0,  1.0), glm::vec2( 0.0, 1.0 ) },
		{ glm::vec3( 1.0,    0.0,  1.0), glm::vec2( 1.0, 1.0 ) },
		{ glm::vec3( 0.0, SQRT_2,  0.0), glm::vec2( 0.5, 1.0 ) },
		{ glm::vec3( 0.0, SQRT_2,  0.0), glm::vec2( 0.5, 1.0 ) },
		{ glm::vec3( 0.0, SQRT_2,  0.0), glm::vec2( 0.5, 1.0 ) },
		{ glm::vec3( 0.0, SQRT_2,  0.0), glm::vec2( 0.5, 1.0 ) },
		{ glm::vec3( 1.0,    0.0, -1.0), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3(-1.0,    0.0, -1.0), glm::vec2( 1.0, 0.0 ) },
		{ glm::vec3(-1.0,    0.0, -1.0), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3(-1.0,    0.0,  1.0), glm::vec2( 1.0, 0.0 ) },
		{ glm::vec3(-1.0,    0.0,  1.0), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3( 1.0,    0.0,  1.0), glm::vec2( 1.0, 0.0 ) },
		{ glm::vec3( 1.0,    0.0,  1.0), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3( 1.0,    0.0, -1.0), glm::vec2( 1.0, 0.0 ) }
	};

	pyramidMeshCPU.indexArray =
	{
		// 1. háromszög
		0,1,2,
		// 2. háromszög
		2,1,3,
		// 3. háromszög
		4,8,9,
		// 4. háromszög
		5,10,11,
		// 5. háromszög
		6,12,13,
		// 6.háromszög
		7,14,15
	};

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( VertexPosTex, position ), 3, GL_FLOAT },
		{ 1, offsetof( VertexPosTex, texcoord ), 2, GL_FLOAT },
	};

	m_pyramidGPU = CreateGLObjectFromMesh( pyramidMeshCPU, vertexAttribList );

}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_pyramidGPU );
}

void CMyApp::InitTextures()
{
	// pyramid

	// diffuse texture
	glGenTextures( 1, &m_TextureID );
	TextureFromFile( m_TextureID, "Assets/yellowbrick.png" );
	SetupTextureSampling( GL_TEXTURE_2D, m_TextureID );

	glGenTextures(1, &m_TextureID2);
	TextureFromFile(m_TextureID2, "Assets/helldivers2.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_TextureID2);
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_TextureID );
	glDeleteTextures( 1, &m_TextureID2);
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
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

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
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


	// pyramid

	// - VAO beállítása
	glBindVertexArray( m_pyramidGPU.vaoID );

	// - Textúrák beállítása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_TextureID );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureID2);

	// - shader bekapcsolasa
	glUseProgram( m_programID );

	// - Uniform paraméterek

	// view és projekciós mátrix
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// Transzformációs mátrixok
	glm::mat4 matWorld = glm::identity<glm::mat4>();

	glUniformMatrix4fv( ul("world"),    1, GL_FALSE, glm::value_ptr( matWorld ) );

	glUniform1i( ul( "texImage" ), 0 );
	glUniform1i( ul( "texImage2" ), 1);

	// Rajzolási parancs kiadása
	glDrawElements( GL_TRIANGLES,    
					m_pyramidGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );

	// shader kikapcsolasa
	glUseProgram( 0 );

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

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