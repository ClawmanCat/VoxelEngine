# Entity Component System
The engine includes an entity component system, roughly based on the EnTT library (https://github.com/skypjack/entt).  
Instead of having one entity class from which all objects in the world derive, each entity is simply an unique ID,
which can have data assigned to it in the form of components at runtime. Systems are classes that iterate over all instances of one or more component types, to provide functionality to these entities.

### Basic Usage
For pretty much all basic uses, interaction with the ECS happens through the `registry`. The registry provides a mapping of each entity to its components and keeps track of systems that iterate over these components.

###### Entities & Components
TODO

###### Systems
TODO

### Queries
It is possible to perform arbitrary queries to create views from the ECS. This is done by combining one or more instances of `ve::ecs::query::has` in combination with the logical operators `&&`, `||` and `!`:
```c++
using namespace ve::ecs;
using namespace ve::ecs::query;

registry reg;
auto view = reg.query(has<Type1> && (has<Type2> || has<Type3>) && !has<Type4>);

for (const auto [entt, t1, t2, t3] : view) {
    t1.do_thing();
    const bool has_t2 = (t2 != nullptr);
    const bool has_t3 = (t3 != nullptr);
}
```
The `value_type` of the view is a tuple with elements automatically deduced from the query:
- Components that are always present for viewed entities (e.g. `Type1` in the above example) are included as references.
- Components that are not guaranteed to be present for viewed entities (e.g. `Type2` and `Type3` in the above example) are included as pointers.
- Components that are used for excluding entities are not present in the value type of the view.

It is also possible to use the constants `ve::ecs::query::true_query` and `ve::ecs::query::false_query` to serve as `true` and `false` constants respectively:
```c++
using namespace ve::ecs;
using namespace ve::ecs::query;

registry reg;
auto view = reg.query(has<Type1> && (has<Type2> || has<Type3> || true_query));

for (const auto [entt, t1, t2, t3] : view) { ... }
```
The above example will create a view over all entities with a component of `Type1`, and also includes `Type2` and `Type3` components if they are present,
but does not exclude entities if they lack both `Type2` and `Type3` components.

> Note:  
> Fold expressions of the form `has<Components> && ...` or `has<Components> || ...` do not work because C++ implicitly inserts `... && true` and `... || false` respectively to handle the case of an empty pack.  
> To work around this, simply insert the default values manually using `true_query` or `false_query`, e.g. by using `has<Components> && ... && true_query`.

###### Adapting Queries
In cases where the value type produced by a query is not desirable, it can be adapted into the desired value type using `ve::ecs::query::adapt_query`.
This will adapt the value type of the query, but does not change which entities it iterates:

```c++
using namespace ve::ecs;
using namespace ve::ecs::query;

registry reg;
auto view  = reg.query(has<Type1> && has<Type2> && (has<Type3> || has<Type4>));
auto adapt = adapt_query<
    /* Required: */ meta::pack<Type1>, 
    /* Optional: */ meta::pack<Type3>
>(view);

for (const auto [entt, t1, t3] : adapt) { ... }
```

When adapting a query, the included and optional components of the new value type must be subsets of those of the original query.
For example, a query of `has<Type1> && has<Type2>` cannot be adapted to return instances of `Type3`.  
Note that when wrapping a query with multiple adapters however, the "original query" still means the initial unadapted query, meaning the following is valid:
```c++
auto view     = reg.query(has<Type1> && has<Type2> && has<Type3>);
auto adapt_t1 = adapt_query<meta::pack<Type1>, meta::pack<>>(view);
auto adapt_t2 = adapt_query<meta::pack<Type2>, meta::pack<>>(adapt_t1);
auto adapt_t3 = adapt_query<meta::pack<Type3>, meta::pack<>>(adapt_t2);
```

###### Query Performance
The amount of time required to iterate over the view of query depends on the so-called "view controller" of that query view.
This is a view of a set of entities that is internally used as a starting point for what entities to include in the view.
- If the view has components that are always present for viewed entities, the controlling view is the smallest view of one of these components.
  E.g. for the query `has<Type1> && has<Type2>` it is either the set of entities that have a `Type1` component, or those that have a `Type2` component, whichever is smaller.
- Otherwise, if the view consists of optionally-present components but at least one of these components has to be present
  (E.g. queries like `has<Type1> || has<Type2> || has<Type3>` but not `has<Type1> || true_query` or `has<Type1> || !has<Type2>`), the controlling view is the union of all these optionally-present component sets.
- Otherwise, the controlling view is the set of all entities. This is not ideal for performance, and such views should be avoided if possible.

It is important to note that the ECS does not check if a query has the possibility of ever actually matching any entity when it creates the view controller.
For example, a query like `(has<Type1> || has<Type2>) && !has<Type1> && !has<Type2>` will still use the union of the entities having `Type1` and `Type2` as its view controller, even though this view could never match any entity.

###### Query Indices
In cases where a slow view is used often, it is possible to create an index on that view to make it perform faster:
```c++
using namespace ve::ecs;
using namespace ve::ecs::query;

registry reg;
reg.add_index_for_query(has<Type1> || has<Type2> || has<Type3> || true_query);

// Optimized query
for (const auto [entt, t1, t2, t3] : reg.query(has<Type1> || has<Type2> || has<Type3> || true_query)) { ... }

// Note: Unaffected even though it will always result in the same set!
for (const auto [entt, t3, t2, t1] : reg.query(has<Type3> || has<Type2> || has<Type1> || true_query)) { ... }
```
It is possible to declare an index of one query as a superset of another query. If this is done, the ECS will assume that any entity matched by the subset-query is also matched by the superset-query, allowing any index on the superset-query to be used for the subset-query:
```c++
using namespace ve::ecs;
using namespace ve::ecs::query;

constexpr auto superset_query = has<Type1> || has<Type2> || has<Type3>;
constexpr auto subset_query   = superset_query && (has<Type4> || has<Type5>);

registry reg;
reg.add_index_for_query(superset_query);
reg.declare_superset(superset_query, subset_query);

// Query will use the index on 'superset_query' for faster iteration.
for (const auto [entt, t1, t2, t3, t4] : reg.query(subset_query)) { ... }
```
Care should be taken to assure that any query marked as a superset of another query actually is one, as the ECS currently is unable to validate this. Marking a query as a superset while it is in fact not one could cause entities to be incorrectly excluded from the query marked as a subset.

### Built-in Components
TODO

### Entity Templates
TODO

### ECS Storage Classes
TODO

### Entity, System & Component Traits
TODO