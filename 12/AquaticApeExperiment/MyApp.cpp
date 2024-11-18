#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"

#include <imgui.h>

#include <string>

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

	m_programAxesID = glCreateProgram();
	AssembleProgram(m_programAxesID, "Shaders/Vert_axes.vert", "Shaders/Frag_PosCol.frag");

	m_programTrajID = glCreateProgram();
	AssembleProgram(m_programTrajID, "Shaders/Vert_traj.vert", "Shaders/Frag_PosCol.frag");

	m_programWaterID = glCreateProgram();
	AssembleProgram(m_programWaterID, "Shaders/Vert_water.vert", "Shaders/Frag_Lighting.frag");

	InitSkyboxShaders();
}

void CMyApp::InitSkyboxShaders()
{
	m_programSkyboxID = glCreateProgram();
	AssembleProgram( m_programSkyboxID, "Shaders/Vert_skybox.vert", "Shaders/Frag_skybox.frag" );
}

void CMyApp::CleanShaders()
{
	glDeleteProgram(m_programID);
	glDeleteProgram(m_programAxesID);
	glDeleteProgram( m_programTrajID );
	glDeleteProgram(m_programWaterID);
	CleanSkyboxShaders();
}

void CMyApp::CleanSkyboxShaders()
{
	glDeleteProgram( m_programSkyboxID );
}

struct Param
{
	glm::vec3 GetPos(float u, float v) const noexcept
	{
        return glm::vec3(u, v, 0.0);
    }

	glm::vec3 GetNorm(float u, float v) const noexcept
	{
        return glm::vec3(0.0, 0.0, 1.0);
    }

	glm::vec2 GetTex( float u, float v ) const noexcept
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

	// Suzanne

	MeshObject<Vertex> suzanneMeshCPU = ObjParser::parse("Assets/Suzanne.obj");

	m_SuzanneGPU = CreateGLObjectFromMesh( suzanneMeshCPU, vertexAttribList );

	// Skybox
	InitSkyboxGeometry();

	//Water
	MeshObject<Vertex> waterCPU = GetParamSurfMesh(Param());
	MeshObject<glm::vec2> waterUVCPU;
	waterUVCPU.indexArray = waterCPU.indexArray;
	for (const Vertex& v : waterCPU.vertexArray) {
		waterUVCPU.vertexArray.emplace_back(glm::vec2(v.position.x, v.position.y));
	}
	
	m_waterGPU = CreateGLObjectFromMesh(waterUVCPU, { {0, offsetof(glm::vec2, x), 2, GL_FLOAT} });
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_SuzanneGPU );
	CleanSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry()
{
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
			// hátsó lap
			glm::vec3(-1, -1, -1),
			glm::vec3( 1, -1, -1),
			glm::vec3( 1,  1, -1),
			glm::vec3(-1,  1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3( 1, -1, 1),
			glm::vec3( 1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
			// hátsó lap
			0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_SkyboxGPU = CreateGLObjectFromMesh( skyboxCPU, { { 0, offsetof( glm::vec3,x ), 3, GL_FLOAT } } );
}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject( m_SkyboxGPU );
}

void CMyApp::InitTextures()
{
	// diffuse texture

	glGenTextures( 1, &m_SuzanneTextureID );
	TextureFromFile( m_SuzanneTextureID, "Assets/wood.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_SuzanneTextureID );

	InitSkyboxTextures();
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_SuzanneTextureID );
	CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures()
{
	// skybox texture

	glGenTextures( 1, &m_skyboxTextureID );
	TextureFromFile( m_skyboxTextureID, "Assets/xpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/xneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/ypos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/yneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/zpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
	TextureFromFile( m_skyboxTextureID, "Assets/zneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
	SetupTextureSampling( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false );

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures()
{
	glDeleteTextures( 1, &m_skyboxTextureID );
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
	
	glPointSize( 16.0f ); // nagyobb pontok
	glLineWidth( 4.0f ); // vastagabb vonalak

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
		glm::vec3(0.0, 7.0, 7.0),	// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

	m_cameraManipulator.SetCamera( &m_camera );

	// kontrolpontok
	m_controlPoints.push_back(glm::vec3(-1.0f, 0.0, -1.0f));
	m_controlPoints.push_back(glm::vec3( 1.0f, 0.0,  1.0f));

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

	//Tengelyek
	glUseProgram(m_programAxesID);

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	glDrawArrays(GL_LINES, 0, 6);

	//Trajectory
	glUseProgram(m_programTrajID);

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glUniform3fv(ul("positions"), m_controlPoints.size(), glm::value_ptr(m_controlPoints[0]));

	glUniform3f(ul("color"), 0.5f, 0.5f, 0.5f);
	glDrawArrays(GL_LINE_STRIP, 0, m_controlPoints.size());
	glUniform3f(ul("color"), 1, 0, 1);
	glDrawArrays(GL_POINTS, 0, m_controlPoints.size());

	// Suzanne

	glBindVertexArray( m_SuzanneGPU.vaoID );

	// - Textúrák beállítása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_SuzanneTextureID );


	glUseProgram( m_programID );

	// - Uniform paraméterek

	// view és projekciós mátrix
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// Transzformációs mátrix
	glm::mat4 matWorld = glm::translate(EvaluatePathPosition());

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
					m_SuzanneGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );


	// Water

	glUseProgram(m_programWaterID);
	glBindVertexArray(m_waterGPU.vaoID);

	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::identity<glm::mat4>()))));


	// - Fényforrások beállítása
	glUniform3fv(ul("cameraPos"), 1, glm::value_ptr(m_camera.GetEye()));
	glUniform4fv(ul("lightPos"), 1, glm::value_ptr(m_lightPos));

	glUniform3fv(ul("La"), 1, glm::value_ptr(m_La));
	glUniform3fv(ul("Ld"), 1, glm::value_ptr(m_Ld));
	glUniform3fv(ul("Ls"), 1, glm::value_ptr(m_Ls));

	glUniform1f(ul("lightConstantAttenuation"), m_lightConstantAttenuation);
	glUniform1f(ul("lightLinearAttenuation"), m_lightLinearAttenuation);
	glUniform1f(ul("lightQuadraticAttenuation"), m_lightQuadraticAttenuation);

	// - Anyagjellemzők beállítása
	glUniform3fv(ul("Ka"), 1, glm::value_ptr(m_Ka));
	glUniform3fv(ul("Kd"), 1, glm::value_ptr(m_Kd));
	glUniform3fv(ul("Ks"), 1, glm::value_ptr(m_Ks));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SuzanneTextureID);
	glUniform1i(ul("textImage"), 0);

	glDrawElements(GL_TRIANGLES, m_waterGPU.count, GL_UNSIGNED_INT, nullptr);


	//
	// skybox
	//

	// - VAO
	glBindVertexArray( m_SkyboxGPU.vaoID );

	// - Textura
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID );

	// - Program
	glUseProgram( m_programSkyboxID );

	// - uniform parameterek
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );
	glUniformMatrix4fv( ul("world"),    1, GL_FALSE, glm::value_ptr( glm::translate( m_camera.GetEye() ) ) );

	// - textúraegységek beállítása
	glUniform1i( ul( "skyboxTexture" ), 0 );

	// mentsük el az előző Z-test eredményt, azaz azt a relációt, ami alapján update-eljük a pixelt.
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	// most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli vágósíkokra
	glDepthFunc(GL_LEQUAL);

	// Rajzolási parancs kiadása
	glDrawElements( GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr );

	glDepthFunc(prevDepthFnc);

	// shader kikapcsolasa
	glUseProgram( 0 );

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

	// VAO kikapcsolása
	glBindVertexArray( 0 );
}

void CMyApp::RenderGUI()
{
	//ImGui::ShowDemoWindow();
	if ( ImGui::Begin( "Lighting settings" ) )
	{		
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

		{
			static glm::vec2 lightPosXZ = glm::vec2( 0.0f );
			lightPosXZ = glm::vec2( m_lightPos.x, m_lightPos.z );
			if ( ImGui::SliderFloat2( "Light Position XZ", glm::value_ptr( lightPosXZ ), -1.0f, 1.0f ) )
			{
				float lightPosL2 = lightPosXZ.x * lightPosXZ.x + lightPosXZ.y * lightPosXZ.y;
				if ( lightPosL2 > 1.0f ) // Ha kívülre esne a körön, akkor normalizáljuk
				{
					lightPosXZ /= sqrtf( lightPosL2 );
					lightPosL2 = 1.0f;
				}

				m_lightPos.x = lightPosXZ.x;
				m_lightPos.z = lightPosXZ.y;
				m_lightPos.y = sqrtf( 1.0f - lightPosL2 );
			}
			ImGui::LabelText( "Light Position Y", "%f", m_lightPos.y );
		}
	}
	ImGui::End();

	if (ImGui::Begin("Experiment"))
	{
		// A paramétert szabályozó csúszka
		ImGui::SliderFloat("Parameter", &m_currentParam, 0, (float)(m_controlPoints.size() - 1));

		ImGui::SeparatorText("Control points");

		// A listboxban megjelenítjük a pontokat
		// Legyen a magasssága annyi, hogy MAX_POINT_COUNT elem férjen bele
		// ImGui::GetTextLineHeightWithSpacing segítségével lekérhető egy sor magassága
		if (ImGui::BeginListBox("Control Points", ImVec2(0.0, MAX_POINT_COUNT * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (int i = 0; i < static_cast<const int>(m_controlPoints.size()); ++i)
			{
				const bool is_seleceted = (m_guiCurrentItem == i); // épp ki van-e jelölve?
				if (ImGui::Selectable(std::to_string(i).c_str(), is_seleceted))
				{
					if (i == m_guiCurrentItem) m_guiCurrentItem = -1; // Ha rákattintottunk, akkor szedjük le a kijelölést
					else m_guiCurrentItem = i; // Különben jelöljük ki
				}

				// technikai apróság, nem baj ha lemarad.
				if (is_seleceted)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		// Gombnyomásra új pontot adunk a végére
		if (ImGui::Button("Add")) // Akkor tér vissza true-val, ha rákattintottunk
		{
			if (m_controlPoints.size() < MAX_POINT_COUNT)
			{
				m_controlPoints.push_back(glm::vec3(0.0f));
				m_guiCurrentItem = static_cast<const int>(m_controlPoints.size() - 1); // Az új pontot állítjuk be aktuálisnak
			}
		}

		ImGui::SameLine();

		// Gombnyomásra töröljük a kijelölt pontot
		if (ImGui::Button("Delete"))
		{
			if (!m_controlPoints.empty() && m_guiCurrentItem < m_controlPoints.size() && m_guiCurrentItem != -1) // currentItem valid index?
			{
				m_controlPoints.erase(m_controlPoints.begin() + m_guiCurrentItem); // Iterátoron keresztül tudjuk törölni a kijelölt elemet
				m_guiCurrentItem = -1; // Törölve lett a kijelölés
			}
		}

		// Ha van kijelölt elem, akkor jelenítsük meg a koordinátáit
		// és lehessen szerkeszteni
		if (m_guiCurrentItem < m_controlPoints.size() && m_guiCurrentItem != -1) // currentItem valid index?
		{
			ImGui::SliderFloat3("Coordinates", glm::value_ptr(m_controlPoints[m_guiCurrentItem]), -10, 10);
		}
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

// Pozíció kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathPosition() const
{
	if (m_controlPoints.size() == 0) // Ha nincs pont, akkor visszaadjuk az origót
		return glm::vec3(0);

	const int interval = (const int)m_currentParam; // Melyik két pont között vagyunk?

	if (interval < 0) // Ha a paraméter negatív, akkor a kezdőpontot adjuk vissza
		return m_controlPoints[0];

	if (interval >= m_controlPoints.size() - 1) // Ha a paraméter nagyobb, mint a pontok száma, akkor az utolsó pontot adjuk vissza
		return m_controlPoints[m_controlPoints.size() - 1];

	float localT = m_currentParam - interval; // A paramétert normalizáljuk az aktuális intervallumra

	if (m_controlPoints.size() >= 4 && interval >= 1 && interval < m_controlPoints.size() - 2)
		return EvaluateCatmull(localT, m_controlPoints[interval - 1], m_controlPoints[interval], m_controlPoints[interval + 1], m_controlPoints[interval + 2]);
	
	return glm::mix( m_controlPoints[interval], m_controlPoints[interval + 1], localT ); // Lineárisan interpolálunk a két kontrollpont között
}

glm::vec3 CMyApp::EvaluateCatmull(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) const
{
	return 0.5f * (
		(2.f * p1) +
		(-p0 + p2) * t +
		(2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t * t +
		(-p0 + 3.f * p1 - 3.f * p2 + p3) * t * t * t
		);
}

// Tangens kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathTangent() const
{
	if (m_controlPoints.size() < 2) // Ha nincs elég pont az interpolációhoy, akkor visszaadjuk az x tengelyt
		return glm::vec3(1.0,0.0,0.0);

	int interval = (int)m_currentParam; // Melyik két pont között vagyunk?

	if (interval < 0) // Ha a paraméter negatív, akkor a kezdő intervallumot adjuk vissza
		interval = 0;

	if (interval >= m_controlPoints.size() - 1) // Ha a paraméter nagyobb, mint az intervallumok száma, akkor az utolsót adjuk vissza
		interval = static_cast<int>( m_controlPoints.size() - 2 );

	return glm::normalize(m_controlPoints[interval + 1] - m_controlPoints[interval]);
}
