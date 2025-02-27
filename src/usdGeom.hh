// SPDX-License-Identifier: MIT
// Copyright 2022 - Present, Syoyo Fujita.
//
// UsdGeom
//
// TODO
//
// - [ ] Replace nonstd::optional<T> member to RelationshipProperty or TypedAttribute***<T>
//
#pragma once

#include "prim-types.hh"
#include "value-types.hh"
#include "xform.hh"

namespace tinyusdz {

// From schema definition.
constexpr auto kGPrim = "GPrim";
constexpr auto kGeomCube = "Cube";
constexpr auto kGeomXform = "Xform";
constexpr auto kGeomMesh = "Mesh";
constexpr auto kGeomSubset = "GeomSubset";
constexpr auto kGeomBasisCurves = "BasisCurves";
constexpr auto kGeomNurbsCurves = "NurbsCurves";
constexpr auto kGeomCylinder = "Cylinder";
constexpr auto kGeomCapsule = "Capsule";
constexpr auto kGeomPoints = "Points";
constexpr auto kGeomCone = "Cone";
constexpr auto kGeomSphere = "Sphere";
constexpr auto kGeomCamera = "Camera";
constexpr auto kPointInstancer = "PointInstancer";

constexpr auto kMaterialBinding = "material:binding";
constexpr auto kMaterialBindingCollection = "material:binding:collection";
constexpr auto kMaterialBindingPreview = "material:binding:preview";

struct GPrim;

bool IsSupportedGeomPrimvarType(uint32_t tyid);
bool IsSupportedGeomPrimvarType(const std::string &type_name);
  
//
// GeomPrimvar is a wrapper class for Attribute and indices(for Indexed Primvar)
// - Attribute with `primvars` prefix. e.g. "primvars:
// - Optional: indices.
//
// GeomPrimvar is only constructable from GPrim.
// This class COPIES variable from GPrim when get operation.
//
// Currently read-only operation are well provided. writing feature is not well tested(`set_value` may have issue)
// (If you struggled to ue GeomPrimvar, please operate on `GPrim::props` directly)
//
// Limitation:
// TimeSamples are not supported for indices.
// Also, TimeSamples are not supported both when constructing GeomPrimvar with Typed Attribute value and retriving Attribute value.
//
//
class GeomPrimvar {

 friend GPrim;

 public:
  GeomPrimvar() : _has_value(false) {
  }

  GeomPrimvar(const Attribute &attr) : _attr(attr) {
    _has_value = true;
  }

  // TODO: TimeSamples indices.
  GeomPrimvar(const Attribute &attr, const std::vector<int32_t> &indices) : _attr(attr), _indices(indices) 
  {
    _has_value = true;
  }

  GeomPrimvar(const GeomPrimvar &rhs) {
    _name = rhs._name;
    _attr = rhs._attr;
    _indices = rhs._indices;
    _has_value = rhs._has_value;
    if (rhs._elementSize) {
      _elementSize = rhs._elementSize;
    }

    if (rhs._interpolation) {
      _interpolation = rhs._interpolation;
    }
  }

  GeomPrimvar &operator=(const GeomPrimvar &rhs) {
    _name = rhs._name;
    _attr = rhs._attr;
    _indices = rhs._indices;
    _has_value = rhs._has_value;
    if (rhs._elementSize) {
      _elementSize = rhs._elementSize;
    }

    if (rhs._interpolation) {
      _interpolation = rhs._interpolation;
    }

    return *this;
  }

  ///
  /// For Indexed Primvar(array value + indices)
  ///
  /// equivalent to ComputeFlattened in pxrUSD.
  ///
  /// ```
  /// for i in len(indices):
  ///   dest[i] = values[indices[i]]
  /// ```
  ///
  /// If Primvar does not have indices, return attribute value as is(same with `get_value`).
  ///
  /// Return false when operation failed or if the attribute type is not supported for Indexed Primvar.
  ///
  template <typename T>
  bool flatten_with_indices(T *dst, std::string *err = nullptr);

  template <typename T>
  bool flatten_with_indices(std::vector<T> *dst, std::string *err = nullptr);

  // Generic Value version.
  bool flatten_with_indices(value::Value *dst, std::string *err = nullptr);

  bool has_elementSize() const;
  uint32_t get_elementSize() const;
  
  bool has_interpolation() const;
  Interpolation get_interpolation() const;

  void set_elementSize(uint32_t n) {
    _elementSize = n;
  }

  void set_interpolation(const Interpolation interp) {
    _interpolation = interp;
  }

  const std::vector<int32_t> &get_indices() const { return _indices; }
  bool has_indices() const { return _indices.size(); }

  uint32_t type_id() { return _attr.type_id(); }
  std::string type_name() { return _attr.type_name(); }

  // Name of Primvar. "primvars:" prefix(namespace) is omitted.
  const std::string name() const { return _name; }

  ///
  /// Attribute has value?(Not empty)
  ///
  bool has_value() const {
    return _has_value;
  }

  ///
  /// Get type name of primvar.
  ///
  std::string get_type_name() const {
    if (!_has_value) {
      return "null";
    }
    
    return _attr.type_name();
  }

  ///
  /// Get type id of primvar.
  ///
  uint32_t get_type_id() const {
    if (!_has_value) {
      return value::TYPE_ID_NULL;
    }
    return _attr.type_id();
  }

  ///
  /// Get Attribute value.
  /// TODO: TimeSamples
  ///
  template <typename T>
  bool get_value(T *dst, std::string *err = nullptr);

  bool get_value(value::Value *dst, std::string *err = nullptr);

  ///
  /// Set Attribute value.
  ///
  template <typename T>
  void set_value(const T &val) {
    _attr.set_value(val);
    _has_value = true;
  }

  void set_value(const Attribute &attr) {
    _attr = attr;
    _has_value = true;
  }

  void set_value(const Attribute &&attr) {
    _attr = std::move(attr);
    _has_value = true;
  }

  void set_name(const std::string &name) { _name = name; }

  void set_indices(const std::vector<int32_t> &indices) {
    _indices = indices;
  }

  void set_indices(const std::vector<int32_t> &&indices) {
    _indices = std::move(indices);
  }

  const Attribute &get_attribute() const {
    return _attr;
  }

 private:

  std::string _name;
  bool _has_value{false};
  Attribute _attr;
  std::vector<int32_t> _indices;  // TODO: uint support?

  // Store Attribute meta separately.
  nonstd::optional<uint32_t> _elementSize;
  nonstd::optional<Interpolation> _interpolation;

#if 0 // TODO
  bool get_value(const value::Value *value,
                 const double t = value::TimeCode::Default(),
                 const value::TimeSampleInterpolationType tinterp =
                     value::TimeSampleInterpolationType::Held);
#endif

};

// Geometric Prim. Encapsulates Imagable + Boundable in pxrUSD schema.
// <pxrUSD>/pxr/usd/usdGeom/schema.udsa

struct GPrim : Xformable {
  std::string name;
  Specifier spec{Specifier::Def};

  int64_t parent_id{-1};  // Index to parent node

  std::string prim_type;  // Primitive type(if specified by `def`)

  void set_name(const std::string &name_) {
    name = name_;
  }

  const std::string &get_name() const {
    return name;
  }

  Specifier &specifier() { return spec; }
  const Specifier &specifier() const { return spec; }

  // Gprim

  TypedAttribute<Animatable<Extent>>
      extent;  // bounding extent. When authorized, the extent is the bounding
               // box of whole its children.

  TypedAttributeWithFallback<bool> doubleSided{
      false};  // "uniform bool doubleSided"

  TypedAttributeWithFallback<Orientation> orientation{
      Orientation::RightHanded};  // "uniform token orientation"
  TypedAttributeWithFallback<Animatable<Visibility>> visibility{
      Visibility::Inherited};  // "token visibility"
  TypedAttributeWithFallback<Purpose> purpose{
      Purpose::Default};  // "uniform token purpose"

  // Handy API to get `primvars:displayColor` and `primvars:displayOpacity`
  bool get_displayColor(value::color3f *col, const double t = value::TimeCode::Default(), const value::TimeSampleInterpolationType tinterp = value::TimeSampleInterpolationType::Held);

  bool get_displayOpacity(float *opacity, const double t = value::TimeCode::Default(), const value::TimeSampleInterpolationType tinterp = value::TimeSampleInterpolationType::Held);

  RelationshipProperty proxyPrim;

  // Some frequently used materialBindings
  nonstd::optional<Relationship> materialBinding; // material:binding
  nonstd::optional<Relationship> materialBindingCollection; // material:binding:collection
  nonstd::optional<Relationship> materialBindingPreview; // material:binding:preview

  std::map<std::string, Property> props;

  std::pair<ListEditQual, std::vector<Reference>> references;
  std::pair<ListEditQual, std::vector<Payload>> payload;
  std::map<std::string, VariantSet> variantSet;

  // For GeomPrimvar.

  ///
  /// Get Attribute(+ indices Attribute for Indexed Primvar) with "primvars:" suffix(namespace) in `props`
  ///
  /// @param[in] name Primvar name(`primvars:` prefix omitted. e.g. "normals", "st0", ...)
  /// @param[out] primvar GeomPrimvar output.
  /// @param[out] err Optional Error message(filled when returning false)
  ///
  bool get_primvar(const std::string &name, GeomPrimvar *primvar, std::string *err = nullptr) const;

  ///
  /// Check if primvar exists with given name
  ///
  /// @param[in] name Primvar name(`primvars:` prefix omitted. e.g. "normals", "st0", ...)
  ///
  bool has_primvar(const std::string &name) const;
  
  ///
  /// Return List of Primvar in this GPrim contains.
  ///
  std::vector<GeomPrimvar> get_primvars() const;

  ///
  /// Set Attribute(+ indices Attribute for Indexed Primvar) with "primvars:" suffix(namespace) to `props`
  ///
  /// @param[in] primvar GeomPrimvar
  /// @param[out] err Optional Error message(filled when returning false)
  ///
  /// Returns true when success to add primvar. Return false on error(e.g. `primvar` does not contain valid name).
  ///
  bool set_primvar(const GeomPrimvar &primvar, std::string *err = nullptr);


  ///
  /// Aux infos
  ///
  std::vector<value::token> &primChildrenNames() {
    return _primChildrenNames;
  }

  std::vector<value::token> &propertyNames() {
    return _propertyNames;
  }

  const std::vector<value::token> &primChildrenNames() const {
    return _primChildrenNames;
  }

  const std::vector<value::token> &propertyNames() const {
    return _propertyNames;
  }

  const std::map<std::string, VariantSet> &variantSetList() const {
    return _variantSetMap;
  }

  std::map<std::string, VariantSet> &variantSetList() {
    return _variantSetMap;
  }

  // Prim metadataum.
  PrimMeta meta; // TODO: Move to private 

  const PrimMeta &metas() const {
    return meta;
  }

  PrimMeta &metas() {
    return meta;
  }

 private:

  //bool _valid{true};  // default behavior is valid(allow empty GPrim)

  std::vector<value::token> _primChildrenNames;
  std::vector<value::token> _propertyNames; 

  // For Variants
  std::map<std::string, VariantSet> _variantSetMap;

};

struct Xform : GPrim {
  // Xform() {}
};

// GeomSubset
struct GeomSubset {
  enum class ElementType { Face };

  enum class FamilyType {
    Partition,       // 'partition'
    NonOverlapping,  // 'nonOverlapping'
    Unrestricted,    // 'unrestricted' (fallback)
  };

  std::string name;
  Specifier spec{Specifier::Def};

  int64_t parent_id{-1};  // Index to parent node

  ElementType elementType{ElementType::Face};  // must be face
  FamilyType familyType{FamilyType::Unrestricted};
  nonstd::optional<value::token> familyName;  // "token familyName"

  nonstd::expected<bool, std::string> SetElementType(const std::string &str) {
    if (str == "face") {
      elementType = ElementType::Face;
      return true;
    }

    return nonstd::make_unexpected(
        "Only `face` is supported for `elementType`, but `" + str +
        "` specified");
  }

  nonstd::expected<bool, std::string> SetFamilyType(const std::string &str) {
    if (str == "partition") {
      familyType = FamilyType::Partition;
      return true;
    } else if (str == "nonOverlapping") {
      familyType = FamilyType::NonOverlapping;
      return true;
    } else if (str == "unrestricted") {
      familyType = FamilyType::Unrestricted;
      return true;
    }

    return nonstd::make_unexpected("Invalid `familyType` specified: `" + str +
                                   "`.");
  }

  std::vector<uint32_t> indices;

  std::map<std::string, Property> props;  // custom Properties
  PrimMeta meta;
};

// Polygon mesh geometry
// X11's X.h uses `None` macro, so add extra prefix to `None` enum
struct GeomMesh : GPrim {
  enum class InterpolateBoundary {
    InterpolateBoundaryNone,  // "none"
    EdgeAndCorner,            // "edgeAndCorner"
    EdgeOnly                  // "edgeOnly"
  };

  enum class FaceVaryingLinearInterpolation {
    CornersPlus1,                        // "cornersPlus1"
    CornersPlus2,                        // "cornersPlus2"
    CornersOnly,                         // "cornersOnly"
    Boundaries,                          // "boundaries"
    FaceVaryingLinearInterpolationNone,  // "none"
    All,                                 // "all"
  };

  enum class SubdivisionScheme {
    CatmullClark,           // "catmullClark"
    Loop,                   // "loop"
    Bilinear,               // "bilinear"
    SubdivisionSchemeNone,  // "none"
  };

  //
  // Predefined attribs.
  //
  TypedAttribute<Animatable<std::vector<value::point3f>>> points;  // point3f[]
  TypedAttribute<Animatable<std::vector<value::normal3f>>>
      normals;  // normal3f[] (NOTE: "primvars:normals" are stored in
                // `GPrim::props`)

  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      velocities;  // vector3f[]

  TypedAttribute<Animatable<std::vector<int32_t>>>
      faceVertexCounts;  // int[] faceVertexCounts
  TypedAttribute<Animatable<std::vector<int32_t>>>
      faceVertexIndices;  // int[] faceVertexIndices

  // Make SkelBindingAPI first citizen.
  nonstd::optional<Relationship> skeleton;  // rel skel:skeleton

  //
  // Utility functions
  //

#if 0 // TODO: Remove
  // Initialize GeomMesh by GPrim(prepend references)
  void Initialize(const GPrim &pprim);

  // Update GeomMesh by GPrim(append references)
  void UpdateBy(const GPrim &pprim);
#endif

  ///
  /// @brief Returns `points`.
  ///
  /// @param[in] time Time for TimeSampled `points` data.
  /// @param[in] interp Interpolation type for TimeSampled `points` data
  /// @return points vector(copied). Returns empty when `points` attribute is
  /// not defined.
  ///
  const std::vector<value::point3f> get_points(
      double time = value::TimeCode::Default(),
      value::TimeSampleInterpolationType interp =
          value::TimeSampleInterpolationType::Linear) const;

  ///
  /// @brief Returns normals vector. Precedence order: `primvars:normals` then
  /// `normals`.
  ///
  /// @return normals vector(copied). Returns empty normals vector when neither
  /// `primvars:normals` nor `normals` attribute defined, attribute is a
  /// relation or normals attribute have invalid type(other than `normal3f`).
  ///
  const std::vector<value::normal3f> get_normals(
      double time = value::TimeCode::Default(),
      value::TimeSampleInterpolationType interp =
          value::TimeSampleInterpolationType::Linear) const;

  ///
  /// @brief Get interpolation of `primvars:normals`, then `normals`.
  /// @return Interpolation of normals. `vertex` by defaut.
  ///
  Interpolation get_normalsInterpolation() const;

  ///
  /// @brief Returns `faceVertexCounts`.
  ///
  /// @return face vertex counts vector(copied)
  ///
  const std::vector<int32_t> get_faceVertexCounts() const;

  ///
  /// @brief Returns `faceVertexIndices`.
  ///
  /// @return face vertex indices vector(copied)
  ///
  const std::vector<int32_t> get_faceVertexIndices() const;

  //
  // SubD attribs.
  //
  TypedAttribute<Animatable<std::vector<int32_t>>>
      cornerIndices;  // int[] cornerIndices
  TypedAttribute<Animatable<std::vector<float>>>
      cornerSharpnesses;  // float[] cornerSharpnesses
  TypedAttribute<Animatable<std::vector<int32_t>>>
      creaseIndices;  // int[] creaseIndices
  TypedAttribute<Animatable<std::vector<int32_t>>>
      creaseLengths;  // int[] creaseLengths
  TypedAttribute<Animatable<std::vector<float>>>
      creaseSharpnesses;  // float[] creaseSharpnesses
  TypedAttribute<Animatable<std::vector<int32_t>>>
      holeIndices;  // int[] holeIndices
  TypedAttributeWithFallback<Animatable<InterpolateBoundary>>
      interpolateBoundary{
          InterpolateBoundary::EdgeAndCorner};  // token interpolateBoundary
  TypedAttributeWithFallback<SubdivisionScheme> subdivisionScheme{
      SubdivisionScheme::CatmullClark};  // uniform token subdivisionScheme
  TypedAttributeWithFallback<Animatable<FaceVaryingLinearInterpolation>>
      faceVaryingLinearInterpolation{
          FaceVaryingLinearInterpolation::
              CornersPlus1};  // token faceVaryingLinearInterpolation

  TypedAttribute<std::vector<value::token>> blendShapes; // uniform token[] skel:blendShapes
  nonstd::optional<Relationship> blendShapeTargets; // rel skel:blendShapeTargets (Path[])

  //
  // TODO: Make these primvars first citizen?
  // - int[] primvars:skel:jointIndices
  // - float[] primvars:skel:jointWeights

  //
  // GeomSubset
  //
  // uniform token `subsetFamily:materialBind:familyType`
  GeomSubset::FamilyType materialBindFamilyType{
      GeomSubset::FamilyType::Partition};

  std::vector<GeomSubset> geom_subset_children;

  ///
  /// Validate GeomSubset data whose are attached to this GeomMesh.
  ///
  nonstd::expected<bool, std::string> ValidateGeomSubset();
};

struct GeomCamera : public GPrim {
  enum class Projection {
    Perspective,   // "perspective"
    Orthographic,  // "orthographic"
  };

  enum class StereoRole {
    Mono,   // "mono"
    Left,   // "left"
    Right,  // "right"
  };

  //
  // Properties
  //

  TypedAttribute<Animatable<std::vector<value::float4>>> clippingPlanes; // float4[]
  TypedAttributeWithFallback<Animatable<value::float2>> clippingRange{
      value::float2({0.1f, 1000000.0f})};
  TypedAttributeWithFallback<Animatable<float>> exposure{0.0f};  // in EV
  TypedAttributeWithFallback<Animatable<float>> focalLength{50.0f};
  TypedAttributeWithFallback<Animatable<float>> focusDistance{0.0f};
  TypedAttributeWithFallback<Animatable<float>> horizontalAperture{20.965f};
  TypedAttributeWithFallback<Animatable<float>> horizontalApertureOffset{0.0f};
  TypedAttributeWithFallback<Animatable<float>> verticalAperture{15.2908f};
  TypedAttributeWithFallback<Animatable<float>> verticalApertureOffset{0.0f};
  TypedAttributeWithFallback<Animatable<float>> fStop{
      0.0f};  // 0.0 = no focusing
  TypedAttributeWithFallback<Animatable<Projection>> projection{
      Projection::Perspective};  // "token projection" Animatable

  TypedAttributeWithFallback<StereoRole> stereoRole{
      StereoRole::Mono};  // "uniform token stereoRole"

  TypedAttributeWithFallback<Animatable<double>> shutterClose{
      0.0};  // double shutter:close
  TypedAttributeWithFallback<Animatable<double>> shutterOpen{
      0.0};  // double shutter:open
};

// struct GeomBoundable : GPrim {};

struct GeomCone : public GPrim {
  //
  // Properties
  //
  TypedAttributeWithFallback<Animatable<double>> height{2.0};
  TypedAttributeWithFallback<Animatable<double>> radius{1.0};

  TypedAttribute<Axis> axis;
};

struct GeomCapsule : public GPrim {
  //
  // Properties
  //
  TypedAttributeWithFallback<Animatable<double>> height{2.0};
  TypedAttributeWithFallback<Animatable<double>> radius{0.5};
  TypedAttribute<Axis> axis;  // uniform token axis
};

struct GeomCylinder : public GPrim {
  //
  // Properties
  //
  TypedAttributeWithFallback<Animatable<double>> height{2.0};
  TypedAttributeWithFallback<Animatable<double>> radius{1.0};
  TypedAttribute<Axis> axis;  // uniform token axis
};

struct GeomCube : public GPrim {
  //
  // Properties
  //
  TypedAttributeWithFallback<Animatable<double>> size{2.0};
};

struct GeomSphere : public GPrim {
  //
  // Predefined attribs.
  //
  TypedAttributeWithFallback<Animatable<double>> radius{2.0};
};

//
// Basis Curves(for hair/fur)
//
struct GeomBasisCurves : public GPrim {
  enum class Type {
    Cubic,   // "cubic"(default)
    Linear,  // "linear"
  };

  enum class Basis {
    Bezier,      // "bezier"(default)
    Bspline,     // "bspline"
    CatmullRom,  // "catmullRom"
  };

  enum class Wrap {
    Nonperiodic,  // "nonperiodic"(default)
    Periodic,     // "periodic"
    Pinned,       // "pinned"
  };

  nonstd::optional<Type> type;
  nonstd::optional<Basis> basis;
  nonstd::optional<Wrap> wrap;

  //
  // Predefined attribs.
  //
  TypedAttribute<Animatable<std::vector<value::point3f>>> points;    // point3f
  TypedAttribute<Animatable<std::vector<value::normal3f>>> normals;  // normal3f
  TypedAttribute<Animatable<std::vector<int>>> curveVertexCounts;
  TypedAttribute<Animatable<std::vector<float>>> widths;
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      velocities;  // vector3f
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      accelerations;  // vector3f
};

struct GeomNurbsCurves : public GPrim {

  //
  // Predefined attribs.
  //
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      accelerations; 
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      velocities; 
  TypedAttribute<Animatable<std::vector<int>>>
      curveVertexCounts;
  TypedAttribute<Animatable<std::vector<value::normal3f>>>
      normals; 
  TypedAttribute<Animatable<std::vector<value::point3f>>>
      points; 
  TypedAttribute<Animatable<std::vector<float>>>
      widths; 


  TypedAttribute<Animatable<std::vector<int>>> order;    
  TypedAttribute<Animatable<std::vector<double>>> knots; 
  TypedAttribute<Animatable<std::vector<value::double2>>> ranges;    
  TypedAttribute<Animatable<std::vector<double>>> pointWeights; 
};

//
// Points primitive.
//
struct GeomPoints : public GPrim {
  //
  // Predefined attribs.
  //
  TypedAttribute<Animatable<std::vector<value::point3f>>> points;  // point3f[]
  TypedAttribute<Animatable<std::vector<value::normal3f>>>
      normals;                                            // normal3f[]
  TypedAttribute<Animatable<std::vector<float>>> widths;  // float[]
  TypedAttribute<Animatable<std::vector<int64_t>>>
      ids;  // int64[] per-point ids.
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      velocities;  // vector3f[]
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      accelerations;  // vector3f[]
};

//
// Point instancer(TODO).
//
struct PointInstancer : public GPrim {
  nonstd::optional<Relationship> prototypes;  // rel prototypes

  TypedAttribute<Animatable<std::vector<int32_t>>>
      protoIndices;                                      // int[] protoIndices
  TypedAttribute<Animatable<std::vector<int64_t>>> ids;  // int64[] ids
  TypedAttribute<Animatable<std::vector<value::point3f>>>
      positions;  // point3f[] positions
  TypedAttribute<Animatable<std::vector<value::quath>>>
      orientations;  // quath[] orientations
  TypedAttribute<Animatable<std::vector<value::float3>>>
      scales;  // float3[] scales
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      velocities;  // vector3f[] velocities
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      accelerations;  // vector3f[] accelerations
  TypedAttribute<Animatable<std::vector<value::vector3f>>>
      angularVelocities;  // vector3f[] angularVelocities
  TypedAttribute<Animatable<std::vector<int64_t>>>
      invisibleIds;  // int64[] invisibleIds
};


// import DEFINE_TYPE_TRAIT and DEFINE_ROLE_TYPE_TRAIT
#include "define-type-trait.inc"

namespace value {

// Geom
DEFINE_TYPE_TRAIT(GPrim, kGPrim, TYPE_ID_GPRIM, 1);

DEFINE_TYPE_TRAIT(Xform, kGeomXform, TYPE_ID_GEOM_XFORM, 1);
DEFINE_TYPE_TRAIT(GeomMesh, kGeomMesh, TYPE_ID_GEOM_MESH, 1);
DEFINE_TYPE_TRAIT(GeomBasisCurves, kGeomBasisCurves, TYPE_ID_GEOM_BASIS_CURVES,
                  1);
DEFINE_TYPE_TRAIT(GeomNurbsCurves, kGeomNurbsCurves, TYPE_ID_GEOM_NURBS_CURVES,
                  1);
DEFINE_TYPE_TRAIT(GeomSphere, kGeomSphere, TYPE_ID_GEOM_SPHERE, 1);
DEFINE_TYPE_TRAIT(GeomCube, kGeomCube, TYPE_ID_GEOM_CUBE, 1);
DEFINE_TYPE_TRAIT(GeomCone, kGeomCone, TYPE_ID_GEOM_CONE, 1);
DEFINE_TYPE_TRAIT(GeomCylinder, kGeomCylinder, TYPE_ID_GEOM_CYLINDER, 1);
DEFINE_TYPE_TRAIT(GeomCapsule, kGeomCapsule, TYPE_ID_GEOM_CAPSULE, 1);
DEFINE_TYPE_TRAIT(GeomPoints, kGeomPoints, TYPE_ID_GEOM_POINTS, 1);
DEFINE_TYPE_TRAIT(GeomSubset, kGeomSubset, TYPE_ID_GEOM_GEOMSUBSET, 1);
DEFINE_TYPE_TRAIT(GeomCamera, kGeomCamera, TYPE_ID_GEOM_CAMERA, 1);
DEFINE_TYPE_TRAIT(PointInstancer, kPointInstancer, TYPE_ID_GEOM_POINT_INSTANCER,
                  1);

#undef DEFINE_TYPE_TRAIT
#undef DEFINE_ROLE_TYPE_TRAIT

}  // namespace value

// For geomprimvar template

// NOTE: Some types are not supported on pxrUSD(e.g. string)
#define APPLY_GEOMPRIVAR_TYPE(__FUNC) \
  __FUNC(value::half)                 \
  __FUNC(value::half2)                \
  __FUNC(value::half3)                \
  __FUNC(value::half4)                \
  __FUNC(int)                         \
  __FUNC(value::int2)                 \
  __FUNC(value::int3)                 \
  __FUNC(value::int4)                 \
  __FUNC(uint32_t)                    \
  __FUNC(value::uint2)                \
  __FUNC(value::uint3)                \
  __FUNC(value::uint4)                \
  __FUNC(float)                       \
  __FUNC(value::float2)               \
  __FUNC(value::float3)               \
  __FUNC(value::float4)               \
  __FUNC(double)                      \
  __FUNC(value::double2)              \
  __FUNC(value::double3)              \
  __FUNC(value::double4)              \
  __FUNC(value::matrix2d)             \
  __FUNC(value::matrix3d)             \
  __FUNC(value::matrix4d)             \
  __FUNC(value::quath)                \
  __FUNC(value::quatf)                \
  __FUNC(value::quatd)                \
  __FUNC(value::normal3h)             \
  __FUNC(value::normal3f)             \
  __FUNC(value::normal3d)             \
  __FUNC(value::vector3h)             \
  __FUNC(value::vector3f)             \
  __FUNC(value::vector3d)             \
  __FUNC(value::point3h)              \
  __FUNC(value::point3f)              \
  __FUNC(value::point3d)              \
  __FUNC(value::color3f)              \
  __FUNC(value::color3d)              \
  __FUNC(value::color4f)              \
  __FUNC(value::color4d)              \
  __FUNC(value::texcoord2h)           \
  __FUNC(value::texcoord2f)           \
  __FUNC(value::texcoord2d)           \
  __FUNC(value::texcoord3h)           \
  __FUNC(value::texcoord3f)           \
  __FUNC(value::texcoord3d)

#define EXTERN_TEMPLATE_GET_VALUE(__ty) \
  extern template bool GeomPrimvar::get_value(__ty *dest, std::string *err); \
  extern template bool GeomPrimvar::get_value(std::vector<__ty> *dest, std::string *err); \
  extern template bool GeomPrimvar::flatten_with_indices(std::vector<__ty> *dest, std::string *err);

APPLY_GEOMPRIVAR_TYPE(EXTERN_TEMPLATE_GET_VALUE)

#undef EXTERN_TEMPLATE_GET_VALUE
#undef APPLY_GEOMPRIVAR_TYPE


}  // namespace tinyusdz
