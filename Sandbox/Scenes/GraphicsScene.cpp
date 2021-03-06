#include "GraphicsScene.h"

using namespace Lumos;
using namespace Maths;

GraphicsScene::GraphicsScene(const std::string& SceneName) : Scene(SceneName), m_Terrain(entt::null) {}

GraphicsScene::~GraphicsScene() = default;

void GraphicsScene::OnInit()
{
	Scene::OnInit();
	Application::Instance()->GetSystem<LumosPhysicsEngine>()->SetDampingFactor(0.998f);
	Application::Instance()->GetSystem<LumosPhysicsEngine>()->SetIntegrationType(IntegrationType::RUNGE_KUTTA_4);
	Application::Instance()->GetSystem<LumosPhysicsEngine>()->SetBroadphase(Lumos::CreateRef<Octree>(5, 3, Lumos::CreateRef<SortAndSweepBroadphase>()));

	LoadModels();

	m_SceneBoundingRadius = 200.0f;

    auto environment = m_Registry.create();
    m_Registry.emplace<Graphics::Environment>(environment, "/Textures/cubemap/Arches_E_PineTree", 11, 3072, 4096, ".tga");
    m_Registry.emplace<NameComponent>(environment, "Environment");

	auto lightEntity = m_Registry.create();
	m_Registry.emplace<Graphics::Light>(lightEntity, Maths::Vector3(26.0f, 22.0f, 48.5f), Maths::Vector4(1.0f), 1.3f);
	m_Registry.emplace<Maths::Transform>(lightEntity, Matrix4::Translation(Maths::Vector3(26.0f, 22.0f, 48.5f)) * Maths::Quaternion::LookAt(Maths::Vector3(26.0f, 22.0f, 48.5f), Maths::Vector3::ZERO).RotationMatrix4());
	m_Registry.emplace<NameComponent>(lightEntity, "Light");

	auto cameraEntity = m_Registry.create();
	auto& camera = m_Registry.emplace<Camera>(cameraEntity, 45.0f, 0.1f, 1000.0f, (float) m_ScreenWidth / (float) m_ScreenHeight);
	m_Registry.emplace<NameComponent>(cameraEntity, "Camera");
	camera.SetCameraController(CreateRef<EditorCameraController>());

	auto audioSystem = Application::Instance()->GetSystem<AudioManager>();
	if (audioSystem)
		Application::Instance()->GetSystem<AudioManager>()->SetListener(&camera);

    bool editor = false;

    #ifdef LUMOS_EDITOR
    editor = true;
    #endif
	auto deferredRenderer = new Graphics::ForwardRenderer(m_ScreenWidth, m_ScreenHeight, editor, true);
	auto skyboxRenderer = new Graphics::SkyboxRenderer(m_ScreenWidth, m_ScreenHeight);

    //Can't render to array texture on iPhoneX or older
#ifndef LUMOS_PLATFORM_IOS
    auto shadowRenderer = new Graphics::ShadowRenderer();
	shadowRenderer->SetLightEntity(lightEntity);
    auto shadowLayer = new Layer3D(shadowRenderer);
    Application::Instance()->GetRenderManager()->SetShadowRenderer(shadowRenderer);
    Application::Instance()->PushLayer(shadowLayer);
#endif

	deferredRenderer->SetRenderToGBufferTexture(editor);
	skyboxRenderer->SetRenderToGBufferTexture(editor);

	auto deferredLayer = new Layer3D(deferredRenderer);
	auto skyBoxLayer = new Layer3D(skyboxRenderer);
	Application::Instance()->PushLayer(deferredLayer);
	Application::Instance()->PushLayer(skyBoxLayer);
}

void GraphicsScene::OnUpdate(const TimeStep& timeStep)
{
	Scene::OnUpdate(timeStep);
}

void GraphicsScene::OnCleanupScene()
{
	Scene::OnCleanupScene();
}

void GraphicsScene::LoadModels()
{
	//HeightMap
	m_Terrain = m_Registry.create(); // EntityManager::Instance()->CreateEntity("heightmap");
	m_Registry.emplace<Maths::Transform>(m_Terrain, Matrix4::Scale(Maths::Vector3(1.0f)));
	m_Registry.emplace<TextureMatrixComponent>(m_Terrain, Matrix4::Scale(Maths::Vector3(1.0f, 1.0f, 1.0f)));
	m_Registry.emplace<NameComponent>(m_Terrain, "HeightMap");
    Lumos::Ref<Graphics::Mesh> terrain = Lumos::Ref<Graphics::Mesh>(new Terrain());

	auto material = Lumos::CreateRef<Material>();
	material->LoadMaterial("checkerboard", "/CoreTextures/checkerboard.tga");

	m_Registry.emplace<MaterialComponent>(m_Terrain, material);
	m_Registry.emplace<MeshComponent>(m_Terrain, terrain);

}

int width = 500;
int height = 500;
int lowside = 50;
int lowscale = 10;
float xRand = 1.0f;
float yRand = 150.0f;
float zRand = 1.0f;
float texRandX = 1.0f / 16.0f;
float texRandZ = 1.0f / 16.0f;

void GraphicsScene::OnImGui()
{
    ImGui::Begin("Terrain");

	ImGui::SliderInt("Width", &width, 1, 5000);
	ImGui::SliderInt("Height", &height, 1, 5000);
	ImGui::SliderInt("lowside", &lowside, 1, 300);
	ImGui::SliderInt("lowscale", &lowscale, 1, 300);

	ImGui::SliderFloat("xRand", &xRand, 0.0f, 300.0f);
	ImGui::SliderFloat("yRand", &yRand, 0.0f, 300.0f);
	ImGui::SliderFloat("zRand", &zRand, 0.0f, 300.0f);

	ImGui::InputFloat("texRandX", &texRandX);
	ImGui::InputFloat("texRandZ", &texRandZ);
    
    if(ImGui::Button("Rebuild Terrain"))
    {
		m_Registry.destroy(m_Terrain);

		m_Terrain = m_Registry.create();
		m_Registry.emplace<Maths::Transform>(m_Terrain, Matrix4::Scale(Maths::Vector3(1.0f)));
		m_Registry.emplace<TextureMatrixComponent>(m_Terrain, Matrix4::Scale(Maths::Vector3(1.0f, 1.0f, 1.0f)));
		m_Registry.emplace<NameComponent>(m_Terrain, "HeightMap");
		Lumos::Ref<Graphics::Mesh> terrain = Lumos::Ref<Graphics::Mesh>(new Terrain(width, height, lowside, lowscale, xRand, yRand, zRand, texRandX, texRandZ));

		auto material = Lumos::CreateRef<Material>();
		material->LoadMaterial("checkerboard", "/CoreTextures/checkerboard.tga");

		m_Registry.emplace<MaterialComponent>(m_Terrain, material);
		m_Registry.emplace<MeshComponent>(m_Terrain, terrain);
    }
    
    ImGui::End();
}
