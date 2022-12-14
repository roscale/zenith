// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'lock_screen_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$LockScreenState {
  bool get dragging => throw _privateConstructorUsedError;
  double get dragVelocity => throw _privateConstructorUsedError;
  double get offset => throw _privateConstructorUsedError;
  double get slideDistance => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $LockScreenStateCopyWith<LockScreenState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $LockScreenStateCopyWith<$Res> {
  factory $LockScreenStateCopyWith(
          LockScreenState value, $Res Function(LockScreenState) then) =
      _$LockScreenStateCopyWithImpl<$Res, LockScreenState>;
  @useResult
  $Res call(
      {bool dragging,
      double dragVelocity,
      double offset,
      double slideDistance});
}

/// @nodoc
class _$LockScreenStateCopyWithImpl<$Res, $Val extends LockScreenState>
    implements $LockScreenStateCopyWith<$Res> {
  _$LockScreenStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? dragging = null,
    Object? dragVelocity = null,
    Object? offset = null,
    Object? slideDistance = null,
  }) {
    return _then(_value.copyWith(
      dragging: null == dragging
          ? _value.dragging
          : dragging // ignore: cast_nullable_to_non_nullable
              as bool,
      dragVelocity: null == dragVelocity
          ? _value.dragVelocity
          : dragVelocity // ignore: cast_nullable_to_non_nullable
              as double,
      offset: null == offset
          ? _value.offset
          : offset // ignore: cast_nullable_to_non_nullable
              as double,
      slideDistance: null == slideDistance
          ? _value.slideDistance
          : slideDistance // ignore: cast_nullable_to_non_nullable
              as double,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_LockScreenStateCopyWith<$Res>
    implements $LockScreenStateCopyWith<$Res> {
  factory _$$_LockScreenStateCopyWith(
          _$_LockScreenState value, $Res Function(_$_LockScreenState) then) =
      __$$_LockScreenStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call(
      {bool dragging,
      double dragVelocity,
      double offset,
      double slideDistance});
}

/// @nodoc
class __$$_LockScreenStateCopyWithImpl<$Res>
    extends _$LockScreenStateCopyWithImpl<$Res, _$_LockScreenState>
    implements _$$_LockScreenStateCopyWith<$Res> {
  __$$_LockScreenStateCopyWithImpl(
      _$_LockScreenState _value, $Res Function(_$_LockScreenState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? dragging = null,
    Object? dragVelocity = null,
    Object? offset = null,
    Object? slideDistance = null,
  }) {
    return _then(_$_LockScreenState(
      dragging: null == dragging
          ? _value.dragging
          : dragging // ignore: cast_nullable_to_non_nullable
              as bool,
      dragVelocity: null == dragVelocity
          ? _value.dragVelocity
          : dragVelocity // ignore: cast_nullable_to_non_nullable
              as double,
      offset: null == offset
          ? _value.offset
          : offset // ignore: cast_nullable_to_non_nullable
              as double,
      slideDistance: null == slideDistance
          ? _value.slideDistance
          : slideDistance // ignore: cast_nullable_to_non_nullable
              as double,
    ));
  }
}

/// @nodoc

class _$_LockScreenState implements _LockScreenState {
  const _$_LockScreenState(
      {required this.dragging,
      required this.dragVelocity,
      required this.offset,
      required this.slideDistance});

  @override
  final bool dragging;
  @override
  final double dragVelocity;
  @override
  final double offset;
  @override
  final double slideDistance;

  @override
  String toString() {
    return 'LockScreenState(dragging: $dragging, dragVelocity: $dragVelocity, offset: $offset, slideDistance: $slideDistance)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_LockScreenState &&
            (identical(other.dragging, dragging) ||
                other.dragging == dragging) &&
            (identical(other.dragVelocity, dragVelocity) ||
                other.dragVelocity == dragVelocity) &&
            (identical(other.offset, offset) || other.offset == offset) &&
            (identical(other.slideDistance, slideDistance) ||
                other.slideDistance == slideDistance));
  }

  @override
  int get hashCode =>
      Object.hash(runtimeType, dragging, dragVelocity, offset, slideDistance);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_LockScreenStateCopyWith<_$_LockScreenState> get copyWith =>
      __$$_LockScreenStateCopyWithImpl<_$_LockScreenState>(this, _$identity);
}

abstract class _LockScreenState implements LockScreenState {
  const factory _LockScreenState(
      {required final bool dragging,
      required final double dragVelocity,
      required final double offset,
      required final double slideDistance}) = _$_LockScreenState;

  @override
  bool get dragging;
  @override
  double get dragVelocity;
  @override
  double get offset;
  @override
  double get slideDistance;
  @override
  @JsonKey(ignore: true)
  _$$_LockScreenStateCopyWith<_$_LockScreenState> get copyWith =>
      throw _privateConstructorUsedError;
}
