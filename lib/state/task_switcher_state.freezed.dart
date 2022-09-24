// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'task_switcher_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$TaskSwitcherState {
  bool get inOverview => throw _privateConstructorUsedError;
  double get scale => throw _privateConstructorUsedError;
  bool get disableUserControl =>
      throw _privateConstructorUsedError; // Disables the ability to switch between tasks using gestures.
  bool get areAnimationsPlaying => throw _privateConstructorUsedError;
  BoxConstraints get constraints => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $TaskSwitcherStateCopyWith<TaskSwitcherState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $TaskSwitcherStateCopyWith<$Res> {
  factory $TaskSwitcherStateCopyWith(
          TaskSwitcherState value, $Res Function(TaskSwitcherState) then) =
      _$TaskSwitcherStateCopyWithImpl<$Res>;
  $Res call(
      {bool inOverview,
      double scale,
      bool disableUserControl,
      bool areAnimationsPlaying,
      BoxConstraints constraints});
}

/// @nodoc
class _$TaskSwitcherStateCopyWithImpl<$Res>
    implements $TaskSwitcherStateCopyWith<$Res> {
  _$TaskSwitcherStateCopyWithImpl(this._value, this._then);

  final TaskSwitcherState _value;
  // ignore: unused_field
  final $Res Function(TaskSwitcherState) _then;

  @override
  $Res call({
    Object? inOverview = freezed,
    Object? scale = freezed,
    Object? disableUserControl = freezed,
    Object? areAnimationsPlaying = freezed,
    Object? constraints = freezed,
  }) {
    return _then(_value.copyWith(
      inOverview: inOverview == freezed
          ? _value.inOverview
          : inOverview // ignore: cast_nullable_to_non_nullable
              as bool,
      scale: scale == freezed
          ? _value.scale
          : scale // ignore: cast_nullable_to_non_nullable
              as double,
      disableUserControl: disableUserControl == freezed
          ? _value.disableUserControl
          : disableUserControl // ignore: cast_nullable_to_non_nullable
              as bool,
      areAnimationsPlaying: areAnimationsPlaying == freezed
          ? _value.areAnimationsPlaying
          : areAnimationsPlaying // ignore: cast_nullable_to_non_nullable
              as bool,
      constraints: constraints == freezed
          ? _value.constraints
          : constraints // ignore: cast_nullable_to_non_nullable
              as BoxConstraints,
    ));
  }
}

/// @nodoc
abstract class _$$_TaskSwitcherStateCopyWith<$Res>
    implements $TaskSwitcherStateCopyWith<$Res> {
  factory _$$_TaskSwitcherStateCopyWith(_$_TaskSwitcherState value,
          $Res Function(_$_TaskSwitcherState) then) =
      __$$_TaskSwitcherStateCopyWithImpl<$Res>;
  @override
  $Res call(
      {bool inOverview,
      double scale,
      bool disableUserControl,
      bool areAnimationsPlaying,
      BoxConstraints constraints});
}

/// @nodoc
class __$$_TaskSwitcherStateCopyWithImpl<$Res>
    extends _$TaskSwitcherStateCopyWithImpl<$Res>
    implements _$$_TaskSwitcherStateCopyWith<$Res> {
  __$$_TaskSwitcherStateCopyWithImpl(
      _$_TaskSwitcherState _value, $Res Function(_$_TaskSwitcherState) _then)
      : super(_value, (v) => _then(v as _$_TaskSwitcherState));

  @override
  _$_TaskSwitcherState get _value => super._value as _$_TaskSwitcherState;

  @override
  $Res call({
    Object? inOverview = freezed,
    Object? scale = freezed,
    Object? disableUserControl = freezed,
    Object? areAnimationsPlaying = freezed,
    Object? constraints = freezed,
  }) {
    return _then(_$_TaskSwitcherState(
      inOverview: inOverview == freezed
          ? _value.inOverview
          : inOverview // ignore: cast_nullable_to_non_nullable
              as bool,
      scale: scale == freezed
          ? _value.scale
          : scale // ignore: cast_nullable_to_non_nullable
              as double,
      disableUserControl: disableUserControl == freezed
          ? _value.disableUserControl
          : disableUserControl // ignore: cast_nullable_to_non_nullable
              as bool,
      areAnimationsPlaying: areAnimationsPlaying == freezed
          ? _value.areAnimationsPlaying
          : areAnimationsPlaying // ignore: cast_nullable_to_non_nullable
              as bool,
      constraints: constraints == freezed
          ? _value.constraints
          : constraints // ignore: cast_nullable_to_non_nullable
              as BoxConstraints,
    ));
  }
}

/// @nodoc

class _$_TaskSwitcherState implements _TaskSwitcherState {
  _$_TaskSwitcherState(
      {required this.inOverview,
      required this.scale,
      required this.disableUserControl,
      required this.areAnimationsPlaying,
      required this.constraints});

  @override
  final bool inOverview;
  @override
  final double scale;
  @override
  final bool disableUserControl;
// Disables the ability to switch between tasks using gestures.
  @override
  final bool areAnimationsPlaying;
  @override
  final BoxConstraints constraints;

  @override
  String toString() {
    return 'TaskSwitcherState(inOverview: $inOverview, scale: $scale, disableUserControl: $disableUserControl, areAnimationsPlaying: $areAnimationsPlaying, constraints: $constraints)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_TaskSwitcherState &&
            const DeepCollectionEquality()
                .equals(other.inOverview, inOverview) &&
            const DeepCollectionEquality().equals(other.scale, scale) &&
            const DeepCollectionEquality()
                .equals(other.disableUserControl, disableUserControl) &&
            const DeepCollectionEquality()
                .equals(other.areAnimationsPlaying, areAnimationsPlaying) &&
            const DeepCollectionEquality()
                .equals(other.constraints, constraints));
  }

  @override
  int get hashCode => Object.hash(
      runtimeType,
      const DeepCollectionEquality().hash(inOverview),
      const DeepCollectionEquality().hash(scale),
      const DeepCollectionEquality().hash(disableUserControl),
      const DeepCollectionEquality().hash(areAnimationsPlaying),
      const DeepCollectionEquality().hash(constraints));

  @JsonKey(ignore: true)
  @override
  _$$_TaskSwitcherStateCopyWith<_$_TaskSwitcherState> get copyWith =>
      __$$_TaskSwitcherStateCopyWithImpl<_$_TaskSwitcherState>(
          this, _$identity);
}

abstract class _TaskSwitcherState implements TaskSwitcherState {
  factory _TaskSwitcherState(
      {required final bool inOverview,
      required final double scale,
      required final bool disableUserControl,
      required final bool areAnimationsPlaying,
      required final BoxConstraints constraints}) = _$_TaskSwitcherState;

  @override
  bool get inOverview;
  @override
  double get scale;
  @override
  bool get disableUserControl;
  @override // Disables the ability to switch between tasks using gestures.
  bool get areAnimationsPlaying;
  @override
  BoxConstraints get constraints;
  @override
  @JsonKey(ignore: true)
  _$$_TaskSwitcherStateCopyWith<_$_TaskSwitcherState> get copyWith =>
      throw _privateConstructorUsedError;
}
