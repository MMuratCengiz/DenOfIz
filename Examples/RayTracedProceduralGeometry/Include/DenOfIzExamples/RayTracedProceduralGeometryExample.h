#pragma once

#include <vector>
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Support/RingBuffer.h"
#ifndef UINT
#define UINT uint32_t
#endif
#include <RayTracingHlslCompat.h>

namespace DenOfIz
{
    namespace AnalyticPrimitive
    {
        enum Enum
        {
            AABB = 0,
            Spheres,
            Count
        };
    } // namespace AnalyticPrimitive

    namespace VolumetricPrimitive
    {
        enum Enum
        {
            MetaBalls = 0,
            Count
        };
    } // namespace VolumetricPrimitive

    namespace SignedDistancePrimitive
    {
        enum Enum
        {
            MiniSpheres = 0,
            IntersectedRoundCube,
            SquareTorus,
            TwistedTorus,
            Cog,
            Cylinder,
            FractalPyramid,
            Count
        };
    } // namespace SignedDistancePrimitive

    namespace IntersectionShaderType
    {
        enum Enum
        {
            AnalyticPrimitive = 0,
            VolumetricPrimitive,
            SignedDistancePrimitive,
            Count
        };

        static uint32_t PerPrimitiveTypeCount( const Enum type )
        {
            switch ( type )
            {
            case AnalyticPrimitive:
                return AnalyticPrimitive::Count;
            case VolumetricPrimitive:
                return VolumetricPrimitive::Count;
            case SignedDistancePrimitive:
                return SignedDistancePrimitive::Count;
            default:
                return 0;
            }
        }

        static constexpr uint32_t TotalPrimitiveCount = AnalyticPrimitive::Count + VolumetricPrimitive::Count + SignedDistancePrimitive::Count;
    } // namespace IntersectionShaderType

    static constexpr float c_aabbWidth    = 1.0f;
    static constexpr float c_aabbDistance = 1.0f;

    class RayTracedProceduralGeometryExample final : public IExample
    {
        static constexpr UINT  NUM_BLAS       = 2;
        static constexpr float c_aabbWidth    = 2.0f;
        static constexpr float c_aabbDistance = 2.0f;

        DenOfIz_Texture m_raytracingOutput[ 3 ] = { DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };
        DenOfIz_Buffer  m_vertexBuffer          = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_indexBuffer           = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_aabbBuffer            = DENOFIZ_NULL_HANDLE;

        PrimitiveConstantBuffer                      m_planeMaterialCB{ };
        std::vector<PrimitiveConstantBuffer>         m_aabbMaterials;
        DenOfIz_RingBuffer                           m_sceneConstantRingBuffer          = DENOFIZ_NULL_HANDLE;
        DenOfIz_RingBuffer                           m_aabbPrimitiveAttributeRingBuffer = DENOFIZ_NULL_HANDLE;
        std::vector<std::vector<std::vector<float>>> m_aabbTransformsPerFrame;

        DenOfIz_BottomLevelAS m_triangleAS = DENOFIZ_NULL_HANDLE;
        DenOfIz_BottomLevelAS m_aabbAS     = DENOFIZ_NULL_HANDLE;
        DenOfIz_TopLevelAS    m_topLevelAS = DENOFIZ_NULL_HANDLE;

        DenOfIz_ShaderProgram                m_rayTracingProgram       = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rayTracingRootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_Pipeline                     m_rayTracingPipeline        = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup                    m_rayTracingBindGroups[ 3 ] = { DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };

        int32_t                    m_closestHitTriangleIndex      = 0;
        int32_t                    m_closestHitAABBIndex          = 0;
        int32_t                    m_firstIntersectionShaderIndex = 0;
        DenOfIz_ShaderBindingTable m_shaderBindingTable           = DENOFIZ_NULL_HANDLE;
        DenOfIz_LocalRootSignature m_hgLocalRootSignature         = DENOFIZ_NULL_HANDLE;

        std::vector<DenOfIz_AABBBoundingBox>              m_aabbs;
        std::vector<std::vector<DenOfIz_AABBBoundingBox>> m_aabbPerGeometry;
        double                                            m_animateGeometryTime = 1.0f;
        bool                                              m_animateGeometry     = true;

        void CreateRenderTargets( );
        void CreateResources( );
        void CreateAccelerationStructures( );
        void CreateRayTracingPipeline( );
        void InitializeScene( );
        void InitCamera( ) const;
        void CreateShaderBindingTable( );
        void BuildProceduralGeometryAABBs( );

    public:
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title  = "RayTracedTriangleExample";
            windowDesc.Width  = 1920;
            windowDesc.Height = 1080;
            return windowDesc;
        }

        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void UpdateAABBPrimitiveAttributes( );
        void UpdateAABBPrimitiveAttributesForFrame( uint32_t frameIndex );
        void Quit( ) override;
        ~RayTracedProceduralGeometryExample( ) override;
    };
} // namespace DenOfIz
