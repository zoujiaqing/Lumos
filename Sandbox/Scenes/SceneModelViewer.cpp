#include "SceneModelViewer.h"

using namespace Lumos;
using namespace Maths;

SceneModelViewer::SceneModelViewer(const std::string& SceneName)
	: Scene(SceneName)
{
}

SceneModelViewer::~SceneModelViewer()
{
}

void SceneModelViewer::OnInit()
{
	Scene::OnInit();

	LoadModels();

	auto audioSystem = Application::Instance()->GetSystem<AudioManager>();

    auto environment = m_Registry.create();
    m_Registry.emplace<Graphics::Environment>(environment, "/Textures/cubemap/Arches_E_PineTree", 11, 3072, 4096, ".tga");
    m_Registry.emplace<NameComponent>(environment, "Environment");

    
    auto lightEntity = m_Registry.create();
    m_Registry.emplace<Graphics::Light>(lightEntity, Maths::Vector3(26.0f, 22.0f, 48.5f), Maths::Vector4(1.0f), 1.3f);
    m_Registry.emplace<Maths::Transform>(lightEntity,Matrix4::Translation(Maths::Vector3(26.0f, 22.0f, 48.5f)) * Maths::Quaternion::LookAt(Maths::Vector3(26.0f, 22.0f, 48.5f), Maths::Vector3::ZERO).RotationMatrix4());
	m_Registry.emplace<NameComponent>(lightEntity, "Directional Light");

    auto cameraEntity = m_Registry.create();
    Camera& camera = m_Registry.emplace<Camera>(cameraEntity, -20.0f, 330.0f, Maths::Vector3(-2.5f, 1.3f, 3.8f), 45.0f, 0.1f, 1000.0f, (float) m_ScreenWidth / (float) m_ScreenHeight);
	camera.SetCameraController(CreateRef<EditorCameraController>());

	if (audioSystem)
		Application::Instance()->GetSystem<AudioManager>()->SetListener(&camera);
	m_Registry.emplace<NameComponent>(cameraEntity, "Camera");

	//Temp
	bool editor = false;

#ifdef LUMOS_EDITOR
	editor = true;
#endif

    auto deferredRenderer = new Graphics::DeferredRenderer(m_ScreenWidth, m_ScreenHeight);
    auto skyboxRenderer = new Graphics::SkyboxRenderer(m_ScreenWidth, m_ScreenHeight);

    deferredRenderer->SetRenderToGBufferTexture(editor);
    skyboxRenderer->SetRenderToGBufferTexture(editor);
    
    auto deferredLayer = new Layer3D(deferredRenderer, "Deferred");
    auto skyBoxLayer = new Layer3D(skyboxRenderer, "Skybox");
    Application::Instance()->PushLayer(deferredLayer);
    Application::Instance()->PushLayer(skyBoxLayer);
        
#ifndef LUMOS_PLATFORM_IOS
    auto shadowRenderer = new Graphics::ShadowRenderer();
    shadowRenderer->SetLightEntity(lightEntity);
    auto shadowLayer = new Layer3D(shadowRenderer);
    Application::Instance()->GetRenderManager()->SetShadowRenderer(shadowRenderer);
    Application::Instance()->PushLayer(shadowLayer);
#endif
    
    m_SceneBoundingRadius = 20.0f;
}

void SceneModelViewer::OnUpdate(const TimeStep& timeStep)
{
	Scene::OnUpdate(timeStep);
}

void SceneModelViewer::OnCleanupScene()
{
	Scene::OnCleanupScene();
}

void SceneModelViewer::LoadModels()
{
	std::vector<String> ExampleModelPaths
	{
		"/Meshes/DamagedHelmet/glTF/DamagedHelmet.gltf",
		"/Meshes/Scene/scene.gltf",
		"/Meshes/Spyro/ArtisansHub.obj",
		"/Meshes/Cube/Cube.gltf",
		"/Meshes/KhronosExamples/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
		"/Meshes/KhronosExamples/EnvironmentTest/glTF/EnvironmentTest.gltf",
        "/Meshes/Sponza/sponza.gltf",
        "/Meshes/capsule.glb"
	};

	auto TestObject = ModelLoader::LoadModel(ExampleModelPaths[0], m_Registry);
	m_Registry.get_or_emplace<Maths::Transform>(TestObject, Maths::Matrix4::Scale(Maths::Vector3(1.0f, 1.0f, 1.0f)));

}

void SceneModelViewer::OnImGui()
{
}
