let gulp = require('gulp'),
    sass = require('gulp-sass'),
    autoprefixer = require('gulp-autoprefixer'),
    sourcemaps = require('gulp-sourcemaps'),
    glob = require('gulp-sass-glob');

gulp.task('sass', function () {
  return gulp
    .src('sass/styles.scss')
    .pipe(sourcemaps.init())
    .pipe(glob())
    .pipe(sass({ includePaths: ['./sass'], outputStyle: 'compressed'}).on('error', sass.logError))
    .pipe(autoprefixer('last 2 version'))
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('./css'));
});

gulp.task('default', function(){
    gulp.watch('sass/**/*.scss', ['sass']);
});
