#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>

#include<vector>
#include<string>

// ���w���C�u�����[�̃C���N���[�h
#include <DirectXMath.h>

// D3D�R���p�C���̃C���N���[�h
#include <d3dcompiler.h>

#define DIRECTINPUT_VERSION 0x0800
#include<dinput.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include <DirectXTex.h>

using namespace DirectX;// ���w���C�u�����[�̃C���N���[�h

// �萔�o�b�t�@�p�f�[�^�\���́i�}�e���A���j
struct ConstBufferDataMaterial {
	XMFLOAT4 color; // �F�iRGBA�j
};

// �E�B���h�E�v���V�[�W��
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// ���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg) {
		//�E�B���h�E���j�����ꂽ
		case WM_DESTROY:
			// OS�ɑ΂��āA�A�v���̏I����`����
			PostQuitMessage(0);
			return 0;
	}
	// �W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// �R���\�[���ւ̕����o��
	OutputDebugStringA("Hello,DirectX!!\n");

	// �E�B���h�E�T�C�Y
	const int window_width = 1280;
	const int window_height = 720;

	// �E�B���h�E�N���X�̐ݒ�
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = L"DirectXGame";
	w.hInstance = GetModuleHandle(nullptr);
	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	// �E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	// �E�B���h�E�T�C�Y{X���W Y���W ���� �c��}
	RECT wrc = { 0,0,window_width,window_height };
	// �����ŃT�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,
		L"DirectXGame",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	// �E�B���h�E��\����Ԃɂ���
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{}; // ���b�Z�[�W

	//�E�E DirectX�����������@��������

#ifdef _DEBUG
	// �f�o�b�O���C���[���I���ɂ���
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif


	HRESULT result;
	ID3D12Device* device = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* rtvHeap = nullptr;

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	// �A�_�v�^�[�̗񋓗p
	std::vector <IDXGIAdapter4*>adapters;

	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter4* tmpAdapter = nullptr;

	//�p�t�H�[�}���X���������̂��珇�ɁA�S�ẴA�_�v�^�[��񋓂���
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference
		(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter))
		!= DXGI_ERROR_NOT_FOUND;
		i++) {
		// ���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}

	// �Ó��ȃA�_�v�^��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		// �\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// �f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i];
			break;
		}
	}

	// �Ή����x���̔z�u
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		// �̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			// �f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	// �R�}���h�A���P�[�^�𐶐�
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	// �R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator, nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	// �R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	// �R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	// �X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// �F���̏���
	swapChainDesc.SampleDesc.Count = 1;// �}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;// �o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;// �o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;// �t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// �X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));

	// �f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;// �����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;// ���\��2��

	// �f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	// �o�b�N�o�b�t�@
	std::vector<ID3D12Resource*> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	// �X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď���������
	for (size_t i = 0; i < backBuffers.size(); i++) {
		// �X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		// �f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		// �����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		// �V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		// �����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
	}

	// �t�F���X�̐���
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	// DirectInput�̏�����
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// �L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// ���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// �r�����䃌�x���̃��Z�b�g

	result = keyboard->SetCooperativeLevel(hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	//�E�E DirectX�����������@�����܂�



	//�E�E �`�揉���������@��������

	// ���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv;  // uv���W
	};

	// ���_�f�[�^
	Vertex vertices[] = {
		{{  0.0f, 100.0f, 0.0f}, {0.0f, 1.0f}},// ����
		{{  0.0f,   0.0f, 0.0f}, {0.0f, 0.0f}},// ����
		{{100.0f, 100.0f, 0.0f}, {1.0f, 1.0f}},// �E��
		{{100.0f,   0.0f, 0.0f}, {1.0f, 0.0f}},// �E��
	};

	unsigned short indices[] = {
		0, 1, 2, // �O�p�`1��
		1, 2, 3, // �O�p�`2��
	};

	// ���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�̃f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	// ���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};// �q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;// GPU�ւ̓]���p
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;// ���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// ���_�o�b�t�@�̐���
	ID3D12Resource* vertBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,  // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// GPU��̃o�b�t�@�ɑΉ��������z�������i���C����������j���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	// �S���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i]; // ���W���R�s�[
	}
	vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	// ���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	// ���_1���̃f�[�^�T�C�Y
	vbView.StrideInBytes = sizeof(vertices[0]);

	ID3DBlob* vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
	ID3DBlob* psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ID3DBlob* errorBlob = nullptr; // �G���[�I�u�W�F�N�g

	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicVS.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "vs_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}


	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicPS.hlsl",// �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,// �C���N���[�h�\�ɂ���
		"main", "ps_5_0",// �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,// �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);


	// �G���[�Ȃ�
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		// ���W�ȊO�ɐF�A�e�N�X�`��UV�Ȃǂ�n���ꍇ�͂���ɑ�����
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
	};

	// �C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	// ���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB;// �C���f�b�N�X��񂪓��镪�̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// �C���f�b�N�X�o�b�t�@�̐���
	ID3D12Resource* indexBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	// �C���f�b�N�X�o�b�t�@���}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// �S�Ă��C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}
	// �}�b�s���O����
	indexBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	// �O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// �V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// �T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;// �W���ݒ�

	// ���X�^���C�U�̐ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineDesc.RasterizerState.DepthClipEnable = true;


	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA�S�Ẵ`�����l����`��

	blenddesc.BlendEnable = true;
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// ����������
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	// ���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	// �}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���̑��ݒ�
	pipelineDesc.NumRenderTargets = 1;// �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;// 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;// 1�s�N�Z���ɂ�1��T���v�����O

	// �萔�o�b�t�@�p�f�[�^�\���́i�}�e���A���j
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; //�F�iRBGA�j
	};

	// �萔�o�b�t�@�p�f�[�^�\���́i3D�ύX�s��j
	struct ConstBufferDataTransform {
		XMMATRIX mat; //3D�ύX�s��
	};

	//�E�E�}�e���A���E�E

	// �q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;// GPU�ւ̓]���p
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff; // 256�o�C�g�A���C�������g
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* constBuffMaterial = nullptr;

	// �萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&cbHeapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,// ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial));
	assert(SUCCEEDED(result));

	// �萔�o�b�t�@�̃}�b�s���O
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // �}�b�s���O
	assert(SUCCEEDED(result));

	// �l���������ނƎ����I�ɓ]�������
	constMapMaterial->color = XMFLOAT4(1, 0, 0, 0.5f);

	//�E�E3D�ύX�s��E�E

	ID3D12Resource* constBuffTransform = nullptr;
	ConstBufferDataTransform* constMapTransform = nullptr;

	{
		// �q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;// GPU�ւ̓]���p
		// ���\�[�X�ݒ�
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff; // 256�o�C�g�A���C�������g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// �萔�o�b�t�@�̐���
		result = device->CreateCommittedResource(
			&cbHeapProp, // �q�[�v�ݒ�
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc,// ���\�[�X�ݒ�
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuffTransform));
		assert(SUCCEEDED(result));

		// �萔�o�b�t�@�̃}�b�s���O
		result = constBuffTransform->Map(0, nullptr, (void**)&constMapTransform);
		assert(SUCCEEDED(result));
	}
	// �P�ʍs�����
	constMapTransform->mat = XMMatrixIdentity();

	constMapTransform->mat.r[0].m128_f32[0] = 2.0f / window_width;
	constMapTransform->mat.r[1].m128_f32[1] = -2.0f / window_height;
	constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
	constMapTransform->mat.r[3].m128_f32[1] = 1.0f;
	

	// �������s�N�Z����
	const size_t texWidth = 256;
	// �z��̗v�f��
	const size_t imageDataCount = texWidth * texWidth;
	// �摜�C���[�W�f�[�^�z��
	XMFLOAT4* imageData = new XMFLOAT4[imageDataCount];

	//// �S�s�N�Z���̐F��������
	//for (size_t i = 0; i < imageDataCount; i++) {
	//	imageData[i].x = 0.0f;
	//	imageData[i].y = 1.0f;
	//	imageData[i].z = 0.0f;
	//	imageData[i].w = 1.0f;
	//}

	TexMetadata metadata{};
	ScratchImage scratchImg{};
	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(
		L"Resources/animal_panda_back.png",
		WIC_FLAGS_NONE,
		&metadata, scratchImg);

	ScratchImage mipChain{};
	// �~�j�}�b�v����
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	// �e�N�X�`���o�b�t�@�ݒ�
	// �q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;

	// �e�N�X�`���o�b�t�@����
	ID3D12Resource* texBuff = nullptr;

	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff));

	// �S�~�j�}�b�v�ɂ���
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		// �~�j�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,
			img->pixels,
			(UINT)img->rowPitch,
			(UINT)img->slicePitch
		);
		assert(SUCCEEDED(result));
	}
	

	// ���f�[�^���
	delete[]imageData;

	// SRV�̍ő��
	const size_t kMaxSRVCount = 2056;

	// �f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	// �ݒ������SRV�p�f�X�N���v�^�q�[�v�𐶐�
	ID3D12DescriptorHeap* srvHeap = nullptr;
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	// SRV�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();

	// �V�F�[�_���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;

	// �n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[���쐬
	device->CreateShaderResourceView(texBuff, &srvDesc, srvHandle);

	// �f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	// �萔�o�b�t�@ 0��
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// �萔�o�b�t�@�r���[
	rootParams[0].Descriptor.ShaderRegister = 0;					// �萔�o�b�t�@�ԍ�
	rootParams[0].Descriptor.RegisterSpace = 0;						// �f�t�H���g�l
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// �S�ẴV�F�[�_���猩����
	// �e�N�X�`�����W�X�^ 0��
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	// �萔�o�b�t�@ 1��
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[2].Descriptor.ShaderRegister = 1;
	rootParams[2].Descriptor.RegisterSpace = 0;
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// �e�N�X�`���T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���J��Ԃ� 
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//�c�J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//�S�Ẵ��j�A����
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;									//�~�b�v�}�b�v�ő�
	samplerDesc.MinLOD = 0.0f;												//�~�b�v�}�b�v�ŏ�
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//�s�N�Z���V�F�[�_����̂ݎg�p�\

	// ���[�g�V�O�l�`��
	ID3D12RootSignature* rootSignature;
	// ���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //���[�g�p�����[�^�̐�[�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams);		//���[�g�p�����[�^
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;
	// ���[�g�V�O�l�`���̃V���A���C�Y
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	rootSigBlob->Release();
	// �p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature;

	// �p�C�v���C���e�[�g�̐���
	ID3D12PipelineState* pipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));



	// �Q�[�����[�v
	while (true) {
		// ���b�Z�[�W������H
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);// �L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);// �v���V�[�W���Ƀ��b�Z�[�W�𑗂�
		}

		// X�{�^���ŏI�����b�Z�[�W��������Q�[�����[�v�𔲂���
		if (msg.message == WM_QUIT) {
			break;
		}

		//�E�E DirectX���t���[�������@��������

		//�L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();

		// �S�L�[�̓��͏󋵂��擾
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);

		// ������0�L�[��������Ă�����
		if (key[DIK_0])
		{
			OutputDebugStringA("Hit 0\n"); //�o�̓E�B���h�E�ɁuHit�@�O�v�ƕ\��
		}

		if (key[DIK_SPACE])  //�X�y�[�X�L�[��������Ă�����
		{

		}



		// �o�b�N�o�b�t�@�̔ԍ����擾�i2�Ȃ̂�0�Ԃ�1�ԁj
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		// 1,�@���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex];// �o�b�N�o�b�t�@���w��
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;// �\����Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;// �`���Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		// 2,�@�`���̕ύX
		// �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		// 3,�@��ʃN���A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };// ���ۂ��F
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// 4,�@�`��R�}���h��������

		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;
		viewport.Height = window_height;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);

		// �V�U�[��`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.right = scissorRect.left + window_width;
		scissorRect.top = 0;
		scissorRect.bottom = scissorRect.top + window_height;
		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		// �p�C�v���C���X�e�[�g�ƃ��[�g�V�O�l�`���̐ݒ�R�}���h
		commandList->SetPipelineState(pipelineState);
		commandList->SetGraphicsRootSignature(rootSignature);

		// �v���~�e�B�u�`��̐ݒ�R�}���h
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// ���_�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetVertexBuffers(0, 1, &vbView);

		//�萔�o�b�t�@�r���[�iCBV�j�̐ݒ�R�}���h (0)
		commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());

		//SRV�q�[�v�̐ݒ�R�}���h
		commandList->SetDescriptorHeaps(1, &srvHeap);
		//SRV�q�[�v�̐擪�n���h���擾
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		//SRV�q�[�v�̐擪�ɂ���SRV�����[�g�p�����[�^1�Ԃɐݒ�
		commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetIndexBuffer(&ibView);

		//�萔�o�b�t�@�r���[�iCBV�j�̐ݒ�R�}���h (1)
		commandList->SetGraphicsRootConstantBufferView(2, constBuffTransform->GetGPUVirtualAddress());

		// �`��R�}���h
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);


		// 4,�@�`��R�}���h�����܂�

		// 5,�@���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;// �`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;       // �\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		// ���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		// �R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, commandLists);

		// �摜�ɕ\������o�b�t�@���t���b�v�i�����̓���ւ��j
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		// �R�}���h�̎��s������҂�
		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		// �L���[���N���A
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		// �ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(commandAllocator, nullptr);
		assert(SUCCEEDED(result));







		//�E�E �摜����ւ�

	}

	// WindowsAPI��n��

	// �E�B���h�E�N���X��o�^����
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}