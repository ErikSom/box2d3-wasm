#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace b2;

// EMSCRIPTEN_DECLARE_VAL_TYPE(b2TaskCallback)
// EMSCRIPTEN_DECLARE_VAL_TYPE(b2EnqueueTaskCallback)


emscripten::val b2ShapeDef_getUserData(const b2ShapeDef& self) {
    return emscripten::val(reinterpret_cast<std::uintptr_t>(self.userData));
}
void b2ShapeDef_setUserData(b2ShapeDef& self, const emscripten::val& value) {
    self.userData = reinterpret_cast<void*>(value.as<std::uintptr_t>());
}

static double b2Filter_getCategoryBits(const b2Filter &f) {
    return (double)f.categoryBits;
}
static void b2Filter_setCategoryBits(b2Filter &f, double val) {
    f.categoryBits = (uint64_t)val;
}
static double b2Filter_getMaskBits(const b2Filter &f) {
    return (double)f.maskBits;
}
static void b2Filter_setMaskBits(b2Filter &f, double val) {
    f.maskBits = (uint64_t)val;
}

template<typename T>
emscripten::val getArrayWrapper(const Body& body, int (Body::*getCount)() const, int (Body::*getData)(T*, int) const) {
    int capacity = (body.*getCount)();
    if (capacity == 0) return emscripten::val::array();

    std::vector<T> items(capacity);
    int count = (body.*getData)(items.data(), capacity);

    auto result = emscripten::val::array();
    for (int i = 0; i < count; i++) {
        result.set(i, items[i]);
    }
    return result;
}

emscripten::val b2ChainDef_getPoints(const b2ChainDef& self) {
    auto result = emscripten::val::array();
    for (int i = 0; i < self.count; i++) {
        auto point = emscripten::val::object();
        point.set("x", self.points[i].x);
        point.set("y", self.points[i].y);
        result.set(i, point);
    }
    return result;
}

EMSCRIPTEN_BINDINGS(box2d) {
    class_<b2Vec2>("b2Vec2")
        .constructor()
        .constructor(+[](float x, float y) -> b2Vec2 { return b2Vec2{x, y}; })
        .property("x", &b2Vec2::x)
        .property("y", &b2Vec2::y)
        ;

    class_<b2CosSin>("b2CosSin")
        .constructor()
        .property("cosine", &b2CosSin::cosine)
        .property("sine", &b2CosSin::sine)
        ;

    class_<b2Rot>("b2Rot")
        .constructor()
        .property("c", &b2Rot::c)
        .property("s", &b2Rot::s)
        ;

    class_<b2Transform>("b2Transform")
        .constructor()
        .property("p", &b2Transform::p, return_value_policy::reference())
        .property("q", &b2Transform::q, return_value_policy::reference())
        ;

    class_<b2Mat22>("b2Mat22")
        .constructor()
        .property("cx", &b2Mat22::cx, return_value_policy::reference())
        .property("cy", &b2Mat22::cy, return_value_policy::reference())
        ;

    class_<b2AABB>("b2AABB")
        .constructor()
        .property("lowerBound", &b2AABB::lowerBound, return_value_policy::reference())
        .property("upperBound", &b2AABB::upperBound, return_value_policy::reference())
        ;

    enum_<b2MixingRule>("b2MixingRule")
        .value("b2_mixAverage", b2MixingRule::b2_mixAverage)
        .value("b2_mixGeometricMean", b2MixingRule::b2_mixGeometricMean)
        .value("b2_mixMultiply", b2MixingRule::b2_mixMultiply)
        .value("b2_mixMinimum", b2MixingRule::b2_mixMinimum)
        .value("b2_mixMaximum", b2MixingRule::b2_mixMaximum)
        ;

    class_<b2WorldDef>("b2WorldDef")
        .constructor()
        .constructor<const b2WorldDef&>()
        .property("gravity", &b2WorldDef::gravity, return_value_policy::reference())
        .property("restitutionThreshold", &b2WorldDef::restitutionThreshold)
        .property("contactPushVelocity", &b2WorldDef::contactPushVelocity)
        .property("hitEventThreshold", &b2WorldDef::hitEventThreshold)
        .property("contactHertz", &b2WorldDef::contactHertz)
        .property("contactDampingRatio", &b2WorldDef::contactDampingRatio)
        .property("jointHertz", &b2WorldDef::jointHertz)
        .property("jointDampingRatio", &b2WorldDef::jointDampingRatio)
        .property("maximumLinearVelocity", &b2WorldDef::maximumLinearVelocity)
        .property("frictionMixingRule", &b2WorldDef::frictionMixingRule)
        .property("restitutionMixingRule", &b2WorldDef::restitutionMixingRule)
        .property("enableSleep", &b2WorldDef::enableSleep)
        .property("enableContinuous", &b2WorldDef::enableContinuous)
        .property("workerCount", &b2WorldDef::workerCount)
        // .property("enqueueTask", &b2WorldDef::enqueueTask, allow_raw_pointers())
        // .property("finishTask", &b2WorldDef::finishTask, allow_raw_pointers())
        // .property("userTaskContext", &b2WorldDef::userTaskContext, allow_raw_pointers())
        // .property("userData", &b2WorldDef::userData, allow_raw_pointers())
        .property("internalValue", &b2WorldDef::internalValue)
        ;

    // reference return policy doesn't work here
    function("b2DefaultWorldDef", &b2DefaultWorldDef);

   class_<b2BodyEvents>("b2BodyEvents")
        .constructor()
        .property("moveEvents", &b2BodyEvents::moveEvents, allow_raw_pointers())
        .property("moveCount", &b2BodyEvents::moveCount)
        ;

    class_<b2BodyMoveEvent>("b2BodyMoveEvent")
        .constructor()
        .property("transform", &b2BodyMoveEvent::transform, return_value_policy::reference())
        .property("bodyId", &b2BodyMoveEvent::bodyId)
        .function("SetUserData", +[](b2BodyMoveEvent& self, const emscripten::val& value) {
            self.userData = reinterpret_cast<void*>(value.as<std::uintptr_t>());
        })
        .function("SetUserData", +[](const b2BodyMoveEvent& self) {
            return emscripten::val(reinterpret_cast<std::uintptr_t>(self.userData));
        })
        .property("fellAsleep", &b2BodyMoveEvent::fellAsleep)
        ;

    class_<b2BodyId>("b2BodyId")
        .constructor()
        .property("index1", &b2BodyId::index1)
        .property("world0", &b2BodyId::world0)
        .property("revision", &b2BodyId::revision)
        ;


    class_<b2ShapeId>("b2ShapeId")
        .constructor()
        .property("index1", &b2ShapeId::index1)
        .property("world0", &b2ShapeId::world0)
        .property("revision", &b2ShapeId::revision)
        ;

    class_<b2WorldId>("b2WorldId")
        .constructor()
        .property("index1", &b2WorldId::index1)
        .property("revision", &b2WorldId::revision)
        ;

    class_<b2ContactEvents>("b2ContactEvents")
        .constructor()
        .property("beginEvents", &b2ContactEvents::beginEvents, allow_raw_pointers())
        .property("endEvents", &b2ContactEvents::endEvents, allow_raw_pointers())
        .property("hitEvents", &b2ContactEvents::hitEvents, allow_raw_pointers())
        .property("beginCount", &b2ContactEvents::beginCount)
        .property("endCount", &b2ContactEvents::endCount)
        .property("hitCount", &b2ContactEvents::hitCount)
        ;


    class_<b2ContactBeginTouchEvent>("b2ContactBeginTouchEvent")
        .constructor()
        .property("shapeIdA", &b2ContactBeginTouchEvent::shapeIdA)
        .property("shapeIdB", &b2ContactBeginTouchEvent::shapeIdB)
        .property("manifold", &b2ContactBeginTouchEvent::manifold, return_value_policy::reference())
        ;


    class_<b2ContactEndTouchEvent>("b2ContactEndTouchEvent")
        .constructor()
        .property("shapeIdA", &b2ContactEndTouchEvent::shapeIdA)
        .property("shapeIdB", &b2ContactEndTouchEvent::shapeIdB)
        ;


    class_<b2ContactHitEvent>("b2ContactHitEvent")
        .constructor()
        .property("shapeIdA", &b2ContactHitEvent::shapeIdA)
        .property("shapeIdB", &b2ContactHitEvent::shapeIdB)
        .property("point", &b2ContactHitEvent::point, return_value_policy::reference())
        .property("normal", &b2ContactHitEvent::normal, return_value_policy::reference())
        .property("approachSpeed", &b2ContactHitEvent::approachSpeed)
        ;


    class_<b2ManifoldPoint>("b2ManifoldPoint")
        .constructor()
        .property("point", &b2ManifoldPoint::point, return_value_policy::reference())
        .property("anchorA", &b2ManifoldPoint::anchorA, return_value_policy::reference())
        .property("anchorB", &b2ManifoldPoint::anchorB, return_value_policy::reference())
        .property("separation", &b2ManifoldPoint::separation)
        .property("normalImpulse", &b2ManifoldPoint::normalImpulse)
        .property("tangentImpulse", &b2ManifoldPoint::tangentImpulse)
        .property("maxNormalImpulse", &b2ManifoldPoint::maxNormalImpulse)
        .property("normalVelocity", &b2ManifoldPoint::normalVelocity)
        .property("id", &b2ManifoldPoint::id)
        .property("persisted", &b2ManifoldPoint::persisted)
        ;

    class_<b2Manifold>("b2Manifold")
        .constructor()
        .function("GetPoint", +[](const b2Manifold& self, int index) -> b2ManifoldPoint {
            return self.points[index];
        })
        .function("SetPoint", +[](b2Manifold& self, int index, const b2ManifoldPoint& point) {
            self.points[index] = point;
        })
        .property("normal", &b2Manifold::normal, return_value_policy::reference())
        .property("pointCount", &b2Manifold::pointCount)
        ;

    class_<b2Counters>("b2Counters")
        .constructor()
        .property("bodyCount", &b2Counters::bodyCount)
        .property("shapeCount", &b2Counters::shapeCount)
        .property("contactCount", &b2Counters::contactCount)
        .property("jointCount", &b2Counters::jointCount)
        .property("islandCount", &b2Counters::islandCount)
        .property("stackUsed", &b2Counters::stackUsed)
        .property("staticTreeHeight", &b2Counters::staticTreeHeight)
        .property("treeHeight", &b2Counters::treeHeight)
        .property("byteCount", &b2Counters::byteCount)
        .property("taskCount", &b2Counters::taskCount)
        .function("GetColorCount", +[](const b2Counters& self, int index) -> int {
            if (index < 0 || index >= 12) {
                return 0;  // Return 0 for out of bounds
            }
            return self.colorCounts[index];
        })
        .function("SetColorCount", +[](b2Counters& self, int index, int value) {
            if (index >= 0 && index < 12) {
                self.colorCounts[index] = value;
            }
        })
        ;

    class_<b2SensorBeginTouchEvent>("b2SensorBeginTouchEvent")
        .constructor()
        .property("sensorShapeId", &b2SensorBeginTouchEvent::sensorShapeId)
        .property("visitorShapeId", &b2SensorBeginTouchEvent::visitorShapeId)
        ;

    class_<b2SensorEndTouchEvent>("b2SensorEndTouchEvent")
        .constructor()
        .property("sensorShapeId", &b2SensorEndTouchEvent::sensorShapeId)
        .property("visitorShapeId", &b2SensorEndTouchEvent::visitorShapeId)
        ;

    class_<b2SensorEvents>("b2SensorEvents")
        .constructor()
        .property("beginEvents", &b2SensorEvents::beginEvents, allow_raw_pointers())
        .property("endEvents", &b2SensorEvents::endEvents, allow_raw_pointers())
        .property("beginCount", &b2SensorEvents::beginCount)
        .property("endCount", &b2SensorEvents::endCount)
        ;

    class_<b2Profile>("b2Profile")
        .constructor()
        .property("step", &b2Profile::step)
        .property("pairs", &b2Profile::pairs)
        .property("collide", &b2Profile::collide)
        .property("solve", &b2Profile::solve)
        .property("buildIslands", &b2Profile::buildIslands)
        .property("solveConstraints", &b2Profile::solveConstraints)
        .property("prepareTasks", &b2Profile::prepareTasks)
        .property("solverTasks", &b2Profile::solverTasks)
        .property("prepareConstraints", &b2Profile::prepareConstraints)
        .property("integrateVelocities", &b2Profile::integrateVelocities)
        .property("warmStart", &b2Profile::warmStart)
        .property("solveVelocities", &b2Profile::solveVelocities)
        .property("integratePositions", &b2Profile::integratePositions)
        .property("relaxVelocities", &b2Profile::relaxVelocities)
        .property("applyRestitution", &b2Profile::applyRestitution)
        .property("storeImpulses", &b2Profile::storeImpulses)
        .property("finalizeBodies", &b2Profile::finalizeBodies)
        .property("splitIslands", &b2Profile::splitIslands)
        .property("sleepIslands", &b2Profile::sleepIslands)
        .property("hitEvents", &b2Profile::hitEvents)
        .property("broadphase", &b2Profile::broadphase)
        .property("continuous", &b2Profile::continuous)
        ;

    class_<BasicWorldInterface<World, false>>("BasicWorldInterface");
    class_<MaybeConstWorldRef<false>>("WorldRef");
    class_<MaybeConstWorldRef<true>>("WorldConstRef");
    class_<World, base<BasicWorldInterface<World, false>>>("World")
        .constructor()
        .constructor<const b2WorldDef&>()
        .function("Destroy", &b2::World::Destroy)
        .function("IsValid", &b2::World::IsValid)
        .function("Step", &b2::World::Step)
        .function("SetGravity", &b2::World::SetGravity)
        .function("GetGravity", &b2::World::GetGravity)
        .function("GetAwakeBodyCount", &b2::World::GetAwakeBodyCount)
        .function("GetBodyEvents", &b2::World::GetBodyEvents)
        .function("GetContactEvents", &b2::World::GetContactEvents)
        .function("SetContactTuning", &b2::World::SetContactTuning)
        .function("EnableContinuous", &b2::World::EnableContinuous)
        .function("IsContinuousEnabled", &b2::World::IsContinuousEnabled)
        .function("EnableSleeping", &b2::World::EnableSleeping)
        .function("IsSleepingEnabled", &b2::World::IsSleepingEnabled)
        .function("EnableWarmStarting", &b2::World::EnableWarmStarting)
        .function("IsWarmStartingEnabled", &b2::World::IsWarmStartingEnabled)
        .function("SetMaximumLinearVelocity", &b2::World::SetMaximumLinearVelocity)
        .function("GetMaximumLinearVelocity", &b2::World::GetMaximumLinearVelocity)
        .function("SetRestitutionThreshold", &b2::World::SetRestitutionThreshold)
        .function("GetRestitutionThreshold", &b2::World::GetRestitutionThreshold)
        .function("SetHitEventThreshold", &b2::World::SetHitEventThreshold)
        .function("GetHitEventThreshold", &b2::World::GetHitEventThreshold)
        .function("GetCounters", &b2::World::GetCounters)
        .function("GetProfile", &b2::World::GetProfile)
        .function("GetSensorEvents", &b2::World::GetSensorEvents)
        .function("SetUserData", +[](b2::World& self, const emscripten::val& value) {
            self.SetUserData(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.as<double>())));
        })
        .function("GetUserData", +[](const b2::World& self) {
            return emscripten::val(reinterpret_cast<std::uintptr_t>(self.GetUserData()));
        })
        .function("CreateBody", +[](b2::World& world, const b2BodyDef& def) -> Body* {
            Body* body = new Body();
            *body = world.CreateBody(b2::Tags::OwningHandle{}, def);
            return body;
        }, allow_raw_pointers())
        ;

    function("b2CreateWorld", &b2CreateWorld, allow_raw_pointers());
    function("b2World_Step", &b2World_Step, allow_raw_pointers());

    // ------------------------------------------------------------------------
    // b2Shape
    // ------------------------------------------------------------------------

     class_<b2Circle>("b2Circle")
        .constructor()
        .property("center", &b2Circle::center)
        .property("radius", &b2Circle::radius)
        ;

    class_<b2Capsule>("b2Capsule")
        .constructor()
        .property("center1", &b2Capsule::center1)
        .property("center2", &b2Capsule::center2)
        .property("radius", &b2Capsule::radius)
        ;

    class_<b2Segment>("b2Segment")
        .constructor()
        .property("point1", &b2Segment::point1)
        .property("point2", &b2Segment::point2)
        ;

    class_<b2Filter>("b2Filter")
        .constructor()
        .property("categoryBits", &b2Filter_getCategoryBits, &b2Filter_setCategoryBits)
        .property("maskBits", &b2Filter_getMaskBits, &b2Filter_setMaskBits)
        .property("groupIndex", &b2Filter::groupIndex);

    class_<b2ShapeDef>("b2ShapeDef")
        .constructor()
        .property("userData", &b2ShapeDef_getUserData, &b2ShapeDef_setUserData)
        .property("friction", &b2ShapeDef::friction)
        .property("restitution", &b2ShapeDef::restitution)
        .property("density", &b2ShapeDef::density)
        .property("filter", &b2ShapeDef::filter)
        .property("customColor", &b2ShapeDef::customColor)
        .property("isSensor", &b2ShapeDef::isSensor)
        .property("enableSensorEvents", &b2ShapeDef::enableSensorEvents)
        .property("enableContactEvents", &b2ShapeDef::enableContactEvents)
        .property("enableHitEvents", &b2ShapeDef::enableHitEvents)
        .property("enablePreSolveEvents", &b2ShapeDef::enablePreSolveEvents)
        .property("invokeContactCreation", &b2ShapeDef::invokeContactCreation)
        .property("updateBodyMass", &b2ShapeDef::updateBodyMass)
        ;

    class_<b2Polygon>("b2Polygon")
        .constructor()
        .function("GetVertex", +[](const b2Polygon& self, int index) -> b2Vec2 {
            if (index < 0 || index >= B2_MAX_POLYGON_VERTICES) return b2Vec2();
            return self.vertices[index];
        })
        .function("SetVertex", +[](b2Polygon& self, int index, const b2Vec2& value) {
            if (index >= 0 && index < B2_MAX_POLYGON_VERTICES) {
                self.vertices[index] = value;
            }
        })
        .function("GetNormal", +[](const b2Polygon& self, int index) -> b2Vec2 {
            if (index < 0 || index >= B2_MAX_POLYGON_VERTICES) return b2Vec2();
            return self.normals[index];
        })
        .function("SetNormal", +[](b2Polygon& self, int index, const b2Vec2& value) {
            if (index >= 0 && index < B2_MAX_POLYGON_VERTICES) {
                self.normals[index] = value;
            }
        })
        .property("centroid", &b2Polygon::centroid)
        .property("radius", &b2Polygon::radius)
        .property("count", &b2Polygon::count)
        .class_function("GetMaxVertices", +[]() { return B2_MAX_POLYGON_VERTICES; })
    ;

    class_<b2ChainDef>("b2ChainDef")
        .constructor<>()
        .function("GetUserData", +[](const b2ChainDef& self) {
            return emscripten::val(reinterpret_cast<std::uintptr_t>(self.userData));
        })
        .function("SetUserData", +[](b2ChainDef& self, const emscripten::val& value) {
            self.userData = reinterpret_cast<void*>(value.as<std::uintptr_t>());
        })
        .property("friction", &b2ChainDef::friction)
        .property("restitution", &b2ChainDef::restitution)
        .property("filter", &b2ChainDef::filter)
        .property("customColor", &b2ChainDef::customColor)
        .property("isLoop", &b2ChainDef::isLoop)
        .function("GetPoints", +[](const b2ChainDef& self) -> emscripten::val {
            auto result = emscripten::val::array();
            for (int i = 0; i < self.count; i++) {
                auto point = emscripten::val::object();
                point.set("x", self.points[i].x);
                point.set("y", self.points[i].y);
                result.set(i, point);
            }
            return result;
        })
        .function("SetPoints", +[](b2ChainDef& self, const emscripten::val& points) {
            int length = points["length"].as<int>();
            b2Vec2* newPoints = new b2Vec2[length];

            for (int i = 0; i < length; i++) {
                auto point = points[i];
                newPoints[i].x = point["x"].as<float>();
                newPoints[i].y = point["y"].as<float>();
            }
            if (self.points != nullptr) {
                delete[] self.points;
            }

            self.points = newPoints;
            self.count = length;
        })
        .property("count", &b2ChainDef::count)
        .property("internalValue", &b2ChainDef::internalValue);

    class_<BasicChainInterface<Chain, false>>("BasicChainInterface");

    class_<Chain, base<BasicChainInterface<Chain, false>>>("Chain")
        .constructor<>()
        .function("Destroy", &Chain::Destroy)
        .function("IsValid", &Chain::IsValid)
        .function("SetFriction", &Chain::SetFriction)
        .function("GetFriction", &Chain::GetFriction)
        .function("SetRestitution", &Chain::SetRestitution)
        .function("GetRestitution", &Chain::GetRestitution)
        .function("GetSegmentCount", &Chain::GetSegmentCount)
        .function("GetSegments", +[](const Chain& self) {
            int count = self.GetSegmentCount();
            if (count == 0) return emscripten::val::array();

            std::vector<b2ShapeId> segments(count);
            int actual = self.GetSegments(segments.data(), count);

            auto result = emscripten::val::array();
            for (int i = 0; i < actual; i++) {
                result.set(i, segments[i]);
            }
            return result;
        })
        .function("GetWorld", select_overload<WorldRef()>(&Chain::GetWorld))
        ;

    function("b2MakeBox", &b2MakeBox);
    function("b2MakeSquare", &b2MakeSquare);
    function("b2MakeRoundedBox", &b2MakeRoundedBox);
    function("b2MakeOffsetBox", &b2MakeOffsetBox);
    function("b2MakeOffsetRoundedBox", &b2MakeOffsetRoundedBox);


    class_<BasicShapeInterface<Shape, false>>("BasicShapeInterface");

    class_<Shape, base<BasicShapeInterface<Shape, false>>>("Shape")
        .constructor()
        .function("Destroy", &Shape::Destroy)
        .function("IsValid", &Shape::IsValid)
        .function("GetAABB", &Shape::GetAABB)
        .function("GetDensity", &Shape::GetDensity)
        .function("SetDensity", &Shape::SetDensity)
        .function("GetFilter", &Shape::GetFilter)
        .function("SetFilter", &Shape::SetFilter)
        .function("GetFriction", &Shape::GetFriction)
        .function("SetFriction", &Shape::SetFriction)
        .function("GetUserData", +[](const Shape& self) {
            return emscripten::val(static_cast<double>(reinterpret_cast<std::uintptr_t>(self.GetUserData())));
        })
        .function("SetUserData", +[](Shape& self, const emscripten::val& value) {
            self.SetUserData(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.as<double>())));
        })
        ;

    function("b2DefaultShapeDef", &b2DefaultShapeDef);
    function("b2CreatePolygonShape", &b2CreatePolygonShape, allow_raw_pointers());
    function("b2CreateCircleShape", &b2CreateCircleShape, allow_raw_pointers());
    function("b2CreateCapsuleShape", &b2CreateCapsuleShape, allow_raw_pointers());
    function("b2CreateSegmentShape", &b2CreateSegmentShape, allow_raw_pointers());

    // ------------------------------------------------------------------------
    // b2Body
    // ------------------------------------------------------------------------
    class_<b2MassData>("b2MassData")
        .constructor()
        .property("mass", &b2MassData::mass)
        .property("center", &b2MassData::center)
        .property("rotationalInertia", &b2MassData::rotationalInertia);
        ;

    enum_<b2BodyType>("b2BodyType")
        .value("b2_staticBody", b2_staticBody)
        .value("b2_kinematicBody", b2_kinematicBody)
        .value("b2_dynamicBody", b2_dynamicBody)
        .value("b2_bodyTypeCount", b2_bodyTypeCount)
        ;

    class_<b2BodyDef>("b2BodyDef")
        .constructor()
        .constructor<const b2BodyDef&>()
        .property("type", &b2BodyDef::type)
        .property("position", &b2BodyDef::position, return_value_policy::reference())
        .property("rotation", &b2BodyDef::rotation, return_value_policy::reference())
        .property("linearVelocity", &b2BodyDef::linearVelocity, return_value_policy::reference())
        .property("angularVelocity", &b2BodyDef::angularVelocity)
        .property("linearDamping", &b2BodyDef::linearDamping)
        .property("angularDamping", &b2BodyDef::angularDamping)
        .property("gravityScale", &b2BodyDef::gravityScale)
        .property("sleepThreshold", &b2BodyDef::sleepThreshold)
        .function("SetUserData", +[](b2BodyDef& self, const emscripten::val& value) {
            self.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.as<double>()));
        })
        .function("GetUserData", +[](const b2BodyDef& self) {
            return emscripten::val(static_cast<double>(reinterpret_cast<std::uintptr_t>(self.userData)));
        })
        .property("enableSleep", &b2BodyDef::enableSleep)
        .property("isAwake", &b2BodyDef::isAwake)
        .property("fixedRotation", &b2BodyDef::fixedRotation)
        .property("isBullet", &b2BodyDef::isBullet)
        .property("isEnabled", &b2BodyDef::isEnabled)
        .property("allowFastRotation", &b2BodyDef::allowFastRotation)
        .property("internalValue", &b2BodyDef::internalValue)
        ;

    function("b2DefaultBodyDef", &b2DefaultBodyDef);

    class_<BasicBodyInterface<Body, false>>("BasicBodyInterface");

    class_<Body, base<BasicBodyInterface<Body, false>>>("Body")
        .constructor<>()
        .function("IsValid", &Body::IsValid)
        .function("CreateChain", +[](Body& body, const b2ChainDef& def) -> Chain* {
            Chain* chain = new Chain();
            *chain = body.CreateChain(Tags::OwningHandle{}, def);
            return chain;
        }, emscripten::allow_raw_pointers())
        .function("CreateShapeCapsule", +[](Body& body, const b2ShapeDef& def, const b2Capsule& capsule) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, capsule);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateShapeCircle", +[](Body& body, const b2ShapeDef& def, const b2Circle& circle) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, circle);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateShapePolygon", +[](Body& body, const b2ShapeDef& def, const b2Polygon& polygon) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, polygon);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateShapeSegment", +[](Body& body, const b2ShapeDef& def, const b2Segment& segment) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, segment);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("Destroy", &Body::Destroy)
        .function("Enable", &Body::Enable)
        .function("Disable", &Body::Disable)
        .function("IsEnabled", &Body::IsEnabled)
        .function("SetAngularDamping", &Body::SetAngularDamping)
        .function("GetAngularDamping", &Body::GetAngularDamping)
        .function("SetAngularVelocity", &Body::SetAngularVelocity)
        .function("GetAngularVelocity", &Body::GetAngularVelocity)
        .function("ApplyAngularImpulse", &Body::ApplyAngularImpulse)
        .function("ApplyForce", &Body::ApplyForce)
        .function("ApplyForceToCenter", &Body::ApplyForceToCenter)
        .function("ApplyLinearImpulse", &Body::ApplyLinearImpulse)
        .function("ApplyLinearImpulseToCenter", &Body::ApplyLinearImpulseToCenter)
        .function("ApplyTorque", &Body::ApplyTorque)
        .function("ApplyMassFromShapes", &Body::ApplyMassFromShapes)
        .function("GetMass", &Body::GetMass)
        .function("SetMassData", &Body::SetMassData)
        .function("GetMassData", &Body::GetMassData)
        .function("GetRotationalInertia", &Body::GetRotationalInertia)
        .function("SetAwake", &Body::SetAwake)
        .function("IsAwake", &Body::IsAwake)
        .function("EnableSleep", &Body::EnableSleep)
        .function("IsSleepEnabled", &Body::IsSleepEnabled)
        .function("SetSleepThreshold", &Body::SetSleepThreshold)
        .function("GetSleepThreshold", &Body::GetSleepThreshold)
        .function("SetBullet", &Body::SetBullet)
        .function("IsBullet", &Body::IsBullet)
        .function("ComputeAABB", &Body::ComputeAABB)
        .function("GetContactCapacity", &Body::GetContactCapacity)
        .function("GetContactData", +[](const Body& body) {
            return getArrayWrapper<b2ContactData>(body, &Body::GetContactCapacity, &Body::GetContactData);
        })
        .function("SetFixedRotation", &Body::SetFixedRotation)
        .function("IsFixedRotation", &Body::IsFixedRotation)
        .function("SetGravityScale", &Body::SetGravityScale)
        .function("GetGravityScale", &Body::GetGravityScale)
        .function("EnableHitEvents", &Body::EnableHitEvents)
        .function("GetJointCount", &Body::GetJointCount)
        .function("GetJoints", +[](const Body& body) {
        return getArrayWrapper<b2JointId>(body, &Body::GetJointCount, &Body::GetJoints);
        })
        .function("SetLinearDamping", &Body::SetLinearDamping)
        .function("GetLinearDamping", &Body::GetLinearDamping)
        .function("SetLinearVelocity", &Body::SetLinearVelocity)
        .function("GetLinearVelocity", &Body::GetLinearVelocity)
        .function("GetLocalCenterOfMass", &Body::GetLocalCenterOfMass)
        .function("GetLocalPoint", &Body::GetLocalPoint)
        .function("GetLocalVector", &Body::GetLocalVector)
        .function("GetPosition", &Body::GetPosition)
        .function("GetRotation", &Body::GetRotation)
        .function("SetTransform", &Body::SetTransform)
        .function("GetTransform", &Body::GetTransform)
        .function("GetShapeCount", &Body::GetShapeCount)
        .function("GetShapes", +[](const Body& body) {
        return getArrayWrapper<b2ShapeId>(body, &Body::GetShapeCount, &Body::GetShapes);
        })
        .function("SetType", &Body::SetType)
        .function("GetType", &Body::GetType)
        .function("GetUserData", +[](const Body& self) {
            return emscripten::val(reinterpret_cast<std::uintptr_t>(self.GetUserData()));
        })
        .function("SetUserData", +[](Body& self, const emscripten::val& value) {
            self.SetUserData(reinterpret_cast<void*>(value.as<std::uintptr_t>()));
        })
        .function("GetWorld", select_overload<WorldRef()>(&Body::GetWorld))
        .function("GetWorldCenterOfMass", &Body::GetWorldCenterOfMass)
        .function("GetWorldPoint", &Body::GetWorldPoint)
        .function("GetWorldVector", &Body::GetWorldVector)
        .function("CreateCapsuleShape", +[](Body& body, const b2ShapeDef& def, const b2Capsule& capsule) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, capsule);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateCircleShape", +[](Body& body, const b2ShapeDef& def, const b2Circle& circle) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, circle);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreatePolygonShape", +[](Body& body, const b2ShapeDef& def, const b2Polygon& polygon) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, polygon);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateChain", +[](Body& body, const b2ChainDef& def) -> Chain* {
            Chain* chain = new Chain();
            *chain = body.CreateChain(Tags::OwningHandle{}, def);
            return chain;
        }, emscripten::allow_raw_pointers())
        ;

    function("b2CreateBody", &b2CreateBody, allow_raw_pointers());
    function("b2Body_GetPosition", &b2Body_GetPosition, allow_raw_pointers());
    function("b2Body_GetRotation", &b2Body_GetRotation, allow_raw_pointers());
    function("b2Rot_GetAngle", &b2Rot_GetAngle, allow_raw_pointers());
}
