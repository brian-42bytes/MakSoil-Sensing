import {
  isRouteErrorResponse,
  useRouteError,
  Link,
} from "react-router-dom";

export default function RouteError() {
  const error = useRouteError();

  let title = "Something went wrong";
  let message = "An unexpected error occurred.";
  let status = 500;

  if (isRouteErrorResponse(error)) {
    status = error.status;
    title = error.statusText || title;
    message = error.data || message;
  } else if (error instanceof Error) {
    message = error.message;
  }

  return (
    <div className="flex min-h-[70vh] items-center justify-center p-6">
      <div className="w-full max-w-xl rounded-2xl border border-red-200 bg-white p-8 shadow-sm">
        <div className="mb-6">
          <p className="text-sm font-semibold uppercase tracking-wide text-red-600">
            Error {status}
          </p>

          <h1 className="mt-2 text-3xl font-bold text-slate-900">
            {title}
          </h1>

          <p className="mt-3 text-slate-600">
            {message}
          </p>
        </div>

        <div className="flex flex-wrap gap-3">
          <Link
            to="/"
            className="rounded-xl bg-slate-900 px-4 py-2 text-sm font-medium text-white hover:bg-slate-800"
          >
            Go Home
          </Link>

          <button
            onClick={() => window.location.reload()}
            className="rounded-xl border border-slate-300 px-4 py-2 text-sm font-medium text-slate-700 hover:bg-slate-100"
          >
            Retry
          </button>
        </div>
      </div>
    </div>
  );
}