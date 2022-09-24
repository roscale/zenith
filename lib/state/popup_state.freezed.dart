// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'popup_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$PopupState {
  int get parentViewId => throw _privateConstructorUsedError;
  Offset get position => throw _privateConstructorUsedError;
  bool get visible => throw _privateConstructorUsedError;
  GlobalKey<AnimationsState> get animationsKey =>
      throw _privateConstructorUsedError;
  bool get isClosing => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $PopupStateCopyWith<PopupState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $PopupStateCopyWith<$Res> {
  factory $PopupStateCopyWith(
          PopupState value, $Res Function(PopupState) then) =
      _$PopupStateCopyWithImpl<$Res>;
  $Res call(
      {int parentViewId,
      Offset position,
      bool visible,
      GlobalKey<AnimationsState> animationsKey,
      bool isClosing});
}

/// @nodoc
class _$PopupStateCopyWithImpl<$Res> implements $PopupStateCopyWith<$Res> {
  _$PopupStateCopyWithImpl(this._value, this._then);

  final PopupState _value;
  // ignore: unused_field
  final $Res Function(PopupState) _then;

  @override
  $Res call({
    Object? parentViewId = freezed,
    Object? position = freezed,
    Object? visible = freezed,
    Object? animationsKey = freezed,
    Object? isClosing = freezed,
  }) {
    return _then(_value.copyWith(
      parentViewId: parentViewId == freezed
          ? _value.parentViewId
          : parentViewId // ignore: cast_nullable_to_non_nullable
              as int,
      position: position == freezed
          ? _value.position
          : position // ignore: cast_nullable_to_non_nullable
              as Offset,
      visible: visible == freezed
          ? _value.visible
          : visible // ignore: cast_nullable_to_non_nullable
              as bool,
      animationsKey: animationsKey == freezed
          ? _value.animationsKey
          : animationsKey // ignore: cast_nullable_to_non_nullable
              as GlobalKey<AnimationsState>,
      isClosing: isClosing == freezed
          ? _value.isClosing
          : isClosing // ignore: cast_nullable_to_non_nullable
              as bool,
    ));
  }
}

/// @nodoc
abstract class _$$_PopupStateCopyWith<$Res>
    implements $PopupStateCopyWith<$Res> {
  factory _$$_PopupStateCopyWith(
          _$_PopupState value, $Res Function(_$_PopupState) then) =
      __$$_PopupStateCopyWithImpl<$Res>;
  @override
  $Res call(
      {int parentViewId,
      Offset position,
      bool visible,
      GlobalKey<AnimationsState> animationsKey,
      bool isClosing});
}

/// @nodoc
class __$$_PopupStateCopyWithImpl<$Res> extends _$PopupStateCopyWithImpl<$Res>
    implements _$$_PopupStateCopyWith<$Res> {
  __$$_PopupStateCopyWithImpl(
      _$_PopupState _value, $Res Function(_$_PopupState) _then)
      : super(_value, (v) => _then(v as _$_PopupState));

  @override
  _$_PopupState get _value => super._value as _$_PopupState;

  @override
  $Res call({
    Object? parentViewId = freezed,
    Object? position = freezed,
    Object? visible = freezed,
    Object? animationsKey = freezed,
    Object? isClosing = freezed,
  }) {
    return _then(_$_PopupState(
      parentViewId: parentViewId == freezed
          ? _value.parentViewId
          : parentViewId // ignore: cast_nullable_to_non_nullable
              as int,
      position: position == freezed
          ? _value.position
          : position // ignore: cast_nullable_to_non_nullable
              as Offset,
      visible: visible == freezed
          ? _value.visible
          : visible // ignore: cast_nullable_to_non_nullable
              as bool,
      animationsKey: animationsKey == freezed
          ? _value.animationsKey
          : animationsKey // ignore: cast_nullable_to_non_nullable
              as GlobalKey<AnimationsState>,
      isClosing: isClosing == freezed
          ? _value.isClosing
          : isClosing // ignore: cast_nullable_to_non_nullable
              as bool,
    ));
  }
}

/// @nodoc

class _$_PopupState with DiagnosticableTreeMixin implements _PopupState {
  const _$_PopupState(
      {required this.parentViewId,
      required this.position,
      required this.visible,
      required this.animationsKey,
      required this.isClosing});

  @override
  final int parentViewId;
  @override
  final Offset position;
  @override
  final bool visible;
  @override
  final GlobalKey<AnimationsState> animationsKey;
  @override
  final bool isClosing;

  @override
  String toString({DiagnosticLevel minLevel = DiagnosticLevel.info}) {
    return 'PopupState(parentViewId: $parentViewId, position: $position, visible: $visible, animationsKey: $animationsKey, isClosing: $isClosing)';
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties
      ..add(DiagnosticsProperty('type', 'PopupState'))
      ..add(DiagnosticsProperty('parentViewId', parentViewId))
      ..add(DiagnosticsProperty('position', position))
      ..add(DiagnosticsProperty('visible', visible))
      ..add(DiagnosticsProperty('animationsKey', animationsKey))
      ..add(DiagnosticsProperty('isClosing', isClosing));
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_PopupState &&
            const DeepCollectionEquality()
                .equals(other.parentViewId, parentViewId) &&
            const DeepCollectionEquality().equals(other.position, position) &&
            const DeepCollectionEquality().equals(other.visible, visible) &&
            const DeepCollectionEquality()
                .equals(other.animationsKey, animationsKey) &&
            const DeepCollectionEquality().equals(other.isClosing, isClosing));
  }

  @override
  int get hashCode => Object.hash(
      runtimeType,
      const DeepCollectionEquality().hash(parentViewId),
      const DeepCollectionEquality().hash(position),
      const DeepCollectionEquality().hash(visible),
      const DeepCollectionEquality().hash(animationsKey),
      const DeepCollectionEquality().hash(isClosing));

  @JsonKey(ignore: true)
  @override
  _$$_PopupStateCopyWith<_$_PopupState> get copyWith =>
      __$$_PopupStateCopyWithImpl<_$_PopupState>(this, _$identity);
}

abstract class _PopupState implements PopupState {
  const factory _PopupState(
      {required final int parentViewId,
      required final Offset position,
      required final bool visible,
      required final GlobalKey<AnimationsState> animationsKey,
      required final bool isClosing}) = _$_PopupState;

  @override
  int get parentViewId;
  @override
  Offset get position;
  @override
  bool get visible;
  @override
  GlobalKey<AnimationsState> get animationsKey;
  @override
  bool get isClosing;
  @override
  @JsonKey(ignore: true)
  _$$_PopupStateCopyWith<_$_PopupState> get copyWith =>
      throw _privateConstructorUsedError;
}
