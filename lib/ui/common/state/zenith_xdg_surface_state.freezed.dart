// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target, unnecessary_question_mark

part of 'zenith_xdg_surface_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$ZenithXdgSurfaceState {
  XdgSurfaceRole get role => throw _privateConstructorUsedError;
  Rect get visibleBounds => throw _privateConstructorUsedError;
  Key get widgetKey => throw _privateConstructorUsedError;
  List<int> get popups => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $ZenithXdgSurfaceStateCopyWith<ZenithXdgSurfaceState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $ZenithXdgSurfaceStateCopyWith<$Res> {
  factory $ZenithXdgSurfaceStateCopyWith(ZenithXdgSurfaceState value,
          $Res Function(ZenithXdgSurfaceState) then) =
      _$ZenithXdgSurfaceStateCopyWithImpl<$Res, ZenithXdgSurfaceState>;
  @useResult
  $Res call(
      {XdgSurfaceRole role,
      Rect visibleBounds,
      Key widgetKey,
      List<int> popups});
}

/// @nodoc
class _$ZenithXdgSurfaceStateCopyWithImpl<$Res,
        $Val extends ZenithXdgSurfaceState>
    implements $ZenithXdgSurfaceStateCopyWith<$Res> {
  _$ZenithXdgSurfaceStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? role = null,
    Object? visibleBounds = null,
    Object? widgetKey = null,
    Object? popups = null,
  }) {
    return _then(_value.copyWith(
      role: null == role
          ? _value.role
          : role // ignore: cast_nullable_to_non_nullable
              as XdgSurfaceRole,
      visibleBounds: null == visibleBounds
          ? _value.visibleBounds
          : visibleBounds // ignore: cast_nullable_to_non_nullable
              as Rect,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
      popups: null == popups
          ? _value.popups
          : popups // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_ZenithXdgSurfaceStateCopyWith<$Res>
    implements $ZenithXdgSurfaceStateCopyWith<$Res> {
  factory _$$_ZenithXdgSurfaceStateCopyWith(_$_ZenithXdgSurfaceState value,
          $Res Function(_$_ZenithXdgSurfaceState) then) =
      __$$_ZenithXdgSurfaceStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call(
      {XdgSurfaceRole role,
      Rect visibleBounds,
      Key widgetKey,
      List<int> popups});
}

/// @nodoc
class __$$_ZenithXdgSurfaceStateCopyWithImpl<$Res>
    extends _$ZenithXdgSurfaceStateCopyWithImpl<$Res, _$_ZenithXdgSurfaceState>
    implements _$$_ZenithXdgSurfaceStateCopyWith<$Res> {
  __$$_ZenithXdgSurfaceStateCopyWithImpl(_$_ZenithXdgSurfaceState _value,
      $Res Function(_$_ZenithXdgSurfaceState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? role = null,
    Object? visibleBounds = null,
    Object? widgetKey = null,
    Object? popups = null,
  }) {
    return _then(_$_ZenithXdgSurfaceState(
      role: null == role
          ? _value.role
          : role // ignore: cast_nullable_to_non_nullable
              as XdgSurfaceRole,
      visibleBounds: null == visibleBounds
          ? _value.visibleBounds
          : visibleBounds // ignore: cast_nullable_to_non_nullable
              as Rect,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
      popups: null == popups
          ? _value._popups
          : popups // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ));
  }
}

/// @nodoc

class _$_ZenithXdgSurfaceState implements _ZenithXdgSurfaceState {
  const _$_ZenithXdgSurfaceState(
      {required this.role,
      required this.visibleBounds,
      required this.widgetKey,
      required final List<int> popups})
      : _popups = popups;

  @override
  final XdgSurfaceRole role;
  @override
  final Rect visibleBounds;
  @override
  final Key widgetKey;
  final List<int> _popups;
  @override
  List<int> get popups {
    if (_popups is EqualUnmodifiableListView) return _popups;
    // ignore: implicit_dynamic_type
    return EqualUnmodifiableListView(_popups);
  }

  @override
  String toString() {
    return 'ZenithXdgSurfaceState(role: $role, visibleBounds: $visibleBounds, widgetKey: $widgetKey, popups: $popups)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_ZenithXdgSurfaceState &&
            (identical(other.role, role) || other.role == role) &&
            (identical(other.visibleBounds, visibleBounds) ||
                other.visibleBounds == visibleBounds) &&
            (identical(other.widgetKey, widgetKey) ||
                other.widgetKey == widgetKey) &&
            const DeepCollectionEquality().equals(other._popups, _popups));
  }

  @override
  int get hashCode => Object.hash(runtimeType, role, visibleBounds, widgetKey,
      const DeepCollectionEquality().hash(_popups));

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_ZenithXdgSurfaceStateCopyWith<_$_ZenithXdgSurfaceState> get copyWith =>
      __$$_ZenithXdgSurfaceStateCopyWithImpl<_$_ZenithXdgSurfaceState>(
          this, _$identity);
}

abstract class _ZenithXdgSurfaceState implements ZenithXdgSurfaceState {
  const factory _ZenithXdgSurfaceState(
      {required final XdgSurfaceRole role,
      required final Rect visibleBounds,
      required final Key widgetKey,
      required final List<int> popups}) = _$_ZenithXdgSurfaceState;

  @override
  XdgSurfaceRole get role;
  @override
  Rect get visibleBounds;
  @override
  Key get widgetKey;
  @override
  List<int> get popups;
  @override
  @JsonKey(ignore: true)
  _$$_ZenithXdgSurfaceStateCopyWith<_$_ZenithXdgSurfaceState> get copyWith =>
      throw _privateConstructorUsedError;
}
