#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"

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
	AssembleProgram( m_programID, "Shaders/Vert_PosNormTex.vert", "Shaders/Frag_Lighting.frag" );

}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
}

// Nyers parameterek
struct Param
{
	glm::vec3 GetPos( float u, float v ) const noexcept
	{
		return glm::vec3( u, v, 0.0f );
	}
	glm::vec3 GetNorm( float u, float v ) const noexcept
	{
		return glm::vec3( 0.0,0.0,1.0 );
	}
	glm::vec2 GetTex( float u, float v ) const noexcept
	{
		return glm::vec2( u, v );
	}
};

struct Sphere 
{
	float r;

	Sphere(float _r = 1.0f) : r(_r) {}

	glm::vec3 GetPos(float u, float v) const noexcept
	{

		u *= glm::two_pi<float>();
		v *= glm::pi<float>();

		return glm::vec3(
			r * sinf(v) * cosf(u),
			r * cosf(v),
			r * sinf(v) * sinf(u)
		);
	}
	glm::vec3 GetNorm(float u, float v) const noexcept //normal vector calculation, for spheres it just happens to be the same as the position calculation
	{
		u *= glm::two_pi<float>();
		v *= glm::two_pi<float>();

		return glm::vec3(
			sinf(v) * cosf(u),
			cosf(v),
			sinf(v) * sinf(u)
		);
	}
	glm::vec2 GetTex(float u, float v) const noexcept
	{
		return glm::vec2(u, v);
	}
};

struct Torus
{
	float r;
	float R;

	Torus(float _r = 1.0f, float _R = 2.0f) 
	{
		r = _r;
		R = _R;
	}

	glm::vec3 GetPos(float u, float v) const noexcept
	{

		u *= glm::two_pi<float>();
		v *= glm::two_pi<float>();

		return glm::vec3(
			(R + r * cosf(u)) * cosf(v),
			r * sinf(u),
			(R + r * cosf(u)) * sinf(v)
		);
	}
	glm::vec3 GetNorm(float u, float v) const noexcept //normal vector calculation, for spheres it just happens to be the same as the position calculation
	{

		//analitikus, parcialis derivalas
		/*u *= glm::two_pi<float>();
		v *= glm::two_pi<float>();

		glm::vec3 du = glm::vec3(-sinf(u) * cosf(v), cosf(u), -sinf(u) * sinf(v));
		glm::vec3 dv = glm::vec3(-sinf(v), 0.f, cosf(v));
		return glm::normalize(glm::cross(du, dv));
		*/

		//numerikus kozelites
		glm::vec3 du = GetPos(u + 0.01f, v) - GetPos(u - 0.01, v);
		glm::vec3 dv = GetPos(u, v  + 0.01f) - GetPos(u, v - 0.01f);
		return glm::normalize(glm::cross(du, dv));
	}
	glm::vec2 GetTex(float u, float v) const noexcept
	{
		return glm::vec2(u, v);
	}
};

struct SurfaceOfRevolution 
{
	float ProfileFunc(float t) const noexcept
	{
		return sqrt((1.f-t) *t);
	}

	glm::vec3 GetPos(float u, float v) const noexcept
	{
		u *= glm::two_pi<float>();
		
		float r = ProfileFunc(v);

		return glm::vec3(
			r * cosf(u),
			v,
			-r * sinf(u));
	}

	//hazi: gpu-n ez, std::vector<glm::vec2> vbo; -> csak vs_in_pos a vertex shaderben, vertex shaderben szamold -> getSurfMesh sima Param()-al hivva, index bufferel vec3 pos = getpos; vec3 norm = getnorm; es ott hasznaljuk ezeket

	glm::vec3 GetNorm(float u, float v) const noexcept //normal vector calculation, for spheres it just happens to be the same as the position calculation
	{
		//numerikus kozelites
		glm::vec3 du = GetPos(u + 0.01f, v) - GetPos(u - 0.01, v);
		glm::vec3 dv = GetPos(u, v + 0.01f) - GetPos(u, v - 0.01f);
		return glm::normalize(glm::cross(du, dv));
	}

	glm::vec2 GetTex(float u, float v) const noexcept
	{
		return glm::vec2(u, v);
	}
};

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( Vertex, position ), 3, GL_FLOAT },
		{ 1, offsetof( Vertex, normal   ), 3, GL_FLOAT },
		{ 2, offsetof( Vertex, texcoord ), 2, GL_FLOAT },
	};

	MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh( SurfaceOfRevolution() );
	m_surfaceGPU = CreateGLObjectFromMesh( surfaceMeshCPU, vertexAttribList );
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_surfaceGPU );
}

void CMyApp::InitTextures()
{
	// diffuse texture
	glGenTextures( 1, &m_TextureID );
	TextureFromFile( m_TextureID, "Assets/color_checkerboard.png" );
	SetupTextureSampling( GL_TEXTURE_2D, m_TextureID );
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_TextureID );
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
		glm::vec3(0.0, 7.0, 7.0),// honnan nézzük a színteret	   - eye
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
	m_lightPos = glm::vec4( m_camera.GetEye(), 1.0 );
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - VAO beállítása
	glBindVertexArray( m_surfaceGPU.vaoID );

	// - Textúrák beállítása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_TextureID );

	glUseProgram( m_programID );

	// - Uniform paraméterek

	// view és projekciós mátrix
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// Transzformációs mátrixok
	glm::mat4 matWorld = glm::identity<glm::mat4>();


	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	// - Fényforrások beállítása
	glUniform3fv( ul( "cameraPos" ), 1, glm::value_ptr( m_camera.GetEye() ) );
	glUniform4fv( ul( "lightPos" ),  1, glm::value_ptr( m_lightPos ) );

	glUniform3fv( ul( "La" ),		 1, glm::value_ptr( m_La ) );
	glUniform3fv( ul( "Ld" ),		 1, glm::value_ptr( m_Ld ) );
	glUniform3fv( ul( "Ls" ),		 1, glm::value_ptr( m_Ls ) );

	glUniform1f( ul( "lightConstantAttenuation"	 ), m_lightConstantAttenuation );
	glUniform1f( ul( "lightLinearAttenuation"	 ), m_lightLinearAttenuation   );
	glUniform1f( ul( "lightQuadraticAttenuation" ), m_lightQuadraticAttenuation);

	// - Anyagjellemzők beállítása
	glUniform3fv( ul( "Ka" ),		 1, glm::value_ptr( m_Ka ) );
	glUniform3fv( ul( "Kd" ),		 1, glm::value_ptr( m_Kd ) );
	glUniform3fv( ul( "Ks" ),		 1, glm::value_ptr( m_Ks ) );

	glUniform1f( ul( "Shininess" ),	m_Shininess );


	// - textúraegységek beállítása
	glUniform1i( ul( "texImage" ), 0 );

	// Rajzolási parancs kiadása
	glDrawElements( GL_TRIANGLES,    
					m_surfaceGPU.count,			 
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
	if ( ImGui::Begin( "Lighting settings" ) )
	{
		ImGui::SeparatorText("Settings");
		
		//ImGui::SliderFloat("Shininess", &m_Shininess, 0.0001f, 10.0f );
		ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f" );
		static float Kaf = 1.0f;
		static float Kdf = 1.0f;
		static float Ksf = 1.0f;
		if ( ImGui::SliderFloat( "Ka", &Kaf, 0.0f, 1.0f ) )
		{
			m_Ka = glm::vec3( Kaf );
		}
		if ( ImGui::SliderFloat( "Kd", &Kdf, 0.0f, 1.0f ) )
		{
			m_Kd = glm::vec3( Kdf );
		}
		if ( ImGui::SliderFloat( "Ks", &Ksf, 0.0f, 1.0f ) )
		{
			m_Ks = glm::vec3( Ksf );
		}

		ImGui::SliderFloat( "Constant Attenuation", &m_lightConstantAttenuation, 0.001f, 2.0f );
		ImGui::SliderFloat( "Linear Attenuation", &m_lightLinearAttenuation, 0.001f, 2.0f );
		ImGui::SliderFloat( "Quadratic Attenuation", &m_lightQuadraticAttenuation, 0.001f, 2.0f );	
	}

	ImGui::End();
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